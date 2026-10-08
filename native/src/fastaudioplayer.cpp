/**
 * @file fastaudioplayer.cpp
 * @brief Native Windows audio playback using WASAPI
 * 
 * Real WASAPI implementation for low-latency audio playback.
 */

#include <jni.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys_devpkey.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <cstring>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

// Audio Player State
struct AudioPlayer {
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* audioDevice = nullptr;
    IAudioClient* audioClient = nullptr;
    IAudioRenderClient* renderClient = nullptr;
    
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> isPaused{false};
    std::atomic<float> volume{1.0f};
    
    std::vector<BYTE> audioData;
    WAVEFORMATEX waveFormat{};        // Format used by WASAPI (mix format)
    WAVEFORMATEX sourceFormat{};     // Original WAV file format
    UINT32 bufferFrameCount = 0;
    UINT32 currentPosition = 0;
    UINT32 totalSamples = 0;
    
    std::thread playbackThread;
    
    ~AudioPlayer() {
        stopPlayback();
        releaseResources();
    }
    
    void releaseResources() {
        if (renderClient) {
            renderClient->Release();
            renderClient = nullptr;
        }
        if (audioClient) {
            audioClient->Release();
            audioClient = nullptr;
        }
        if (audioDevice) {
            audioDevice->Release();
            audioDevice = nullptr;
        }
        if (deviceEnumerator) {
            deviceEnumerator->Release();
            deviceEnumerator = nullptr;
        }
    }
    
    void stopPlayback() {
        isPlaying = false;
        isPaused = false;
        if (playbackThread.joinable()) {
            playbackThread.join();
        }
    }
};

// Helper: Convert UTF-8 to wide string
std::wstring UTF8ToWString(JNIEnv* env, jstring str) {
    if (!str) return L"";
    const char* chars = env->GetStringUTFChars(str, nullptr);
    if (!chars) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, chars, -1, nullptr, 0);
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, chars, -1, &result[0], len);
    env->ReleaseStringUTFChars(str, chars);
    return result;
}

// Helper: Convert wide string to jstring
jstring WStringToJString(JNIEnv* env, const std::wstring& wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, nullptr, nullptr);
    return env->NewStringUTF(result.c_str());
}

// Parse WAV in-memory buffer
bool ParseWavMemory(const BYTE* pBuffer, size_t bufferSize, std::vector<BYTE>& audioData, WAVEFORMATEX& format) {
    if (!pBuffer || bufferSize < 44) return false;
    
    // Check "RIFF" and "WAVE" signatures
    if (memcmp(pBuffer, "RIFF", 4) != 0 || memcmp(pBuffer + 8, "WAVE", 4) != 0) {
        return false;
    }
    
    // Parse format
    format.wFormatTag = *(WORD*)(pBuffer + 20);
    format.nChannels = *(WORD*)(pBuffer + 22);
    format.nSamplesPerSec = *(DWORD*)(pBuffer + 24);
    format.nAvgBytesPerSec = *(DWORD*)(pBuffer + 28);
    format.nBlockAlign = *(WORD*)(pBuffer + 32);
    format.wBitsPerSample = *(WORD*)(pBuffer + 34);
    format.cbSize = 0;
    
    if (format.wFormatTag == 0xFFFE && bufferSize >= 64) {
        WORD subFormatLow = *(WORD*)(pBuffer + 44);
        if (subFormatLow == 1) {
            format.wFormatTag = WAVE_FORMAT_PCM;
        }
    }
    
    DWORD fmtChunkSize = *(DWORD*)(pBuffer + 16);
    size_t pos = 12 + 8 + fmtChunkSize;
    if (fmtChunkSize % 2) pos++;
    
    size_t fileSize = *(DWORD*)(pBuffer + 4) + 8;
    if (fileSize > bufferSize) fileSize = bufferSize;
    
    while (pos + 8 <= fileSize) {
        const BYTE* chunkHeader = pBuffer + pos;
        DWORD chunkSize = *(DWORD*)(chunkHeader + 4);
        
        if (memcmp(chunkHeader, "data", 4) == 0) {
            if (pos + 8 + chunkSize > bufferSize) {
                chunkSize = (DWORD)(bufferSize - (pos + 8));
            }
            audioData.assign(pBuffer + pos + 8, pBuffer + pos + 8 + chunkSize);
            break;
        }
        
        pos += 8 + chunkSize;
        if (chunkSize % 2) pos++;
    }
    
    return !audioData.empty();
}

// Parse WAV file header
bool ParseWavFile(const wchar_t* filePath, std::vector<BYTE>& audioData, WAVEFORMATEX& format) {
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size) || size.QuadPart < 44 || size.QuadPart > 500 * 1024 * 1024) {
        CloseHandle(hFile);
        return false;
    }
    
    std::vector<BYTE> fileBuffer(static_cast<size_t>(size.QuadPart));
    DWORD bytesRead;
    if (!ReadFile(hFile, fileBuffer.data(), static_cast<DWORD>(fileBuffer.size()), &bytesRead, nullptr) || bytesRead != fileBuffer.size()) {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    
    return ParseWavMemory(fileBuffer.data(), fileBuffer.size(), audioData, format);
}

// Playback thread function
void PlaybackThread(AudioPlayer* player) {
    HRESULT hr;
    UINT32 numFramesAvailable;
    BYTE* pData;
    DWORD flags = 0;
    
    // Start audio client
    hr = player->audioClient->Start();
    if (FAILED(hr)) {
        player->isPlaying = false;
        return;
    }
    
    while (player->isPlaying) {
        if (player->isPaused) {
            Sleep(10);
            continue;
        }
        
        // Get buffer size / current padding
        UINT32 currentPadding;
        hr = player->audioClient->GetCurrentPadding(&currentPadding);
        if (FAILED(hr)) break;
        
        // If we have written all samples, wait only for the buffered frames to play out
        if (player->currentPosition >= player->totalSamples) {
            if (currentPadding <= (player->waveFormat.nSamplesPerSec / 100)) { // <= 10 ms remaining
                player->isPlaying = false;
                break;
            }
            Sleep(2);
            continue;
        }
        
        numFramesAvailable = player->bufferFrameCount - currentPadding;
        
        if (numFramesAvailable > 0) {
            // Get buffer
            hr = player->renderClient->GetBuffer(numFramesAvailable, &pData);
            if (FAILED(hr)) break;
            if (!pData) break;
            
            // Calculate buffer sizes
            UINT32 dstBytesPerFrame = player->waveFormat.nBlockAlign;  // Output format (could be 32-bit float)
            UINT32 srcBytesPerFrame = player->sourceFormat.nBlockAlign; // Source format (16-bit PCM)
            UINT32 bytesAvailable = numFramesAvailable * dstBytesPerFrame;
            
            double resampleRatio = (double)player->sourceFormat.nSamplesPerSec / (double)player->waveFormat.nSamplesPerSec;
            UINT32 remainingSrcFrames = (player->totalSamples > player->currentPosition) ? (player->totalSamples - player->currentPosition) : 0;
            UINT32 maxDstFramesCanFeed = (resampleRatio > 0) ? (UINT32)(remainingSrcFrames / resampleRatio) : 0;
            UINT32 framesToRender = min(numFramesAvailable, maxDstFramesCanFeed);
            
            if (framesToRender > 0) {
                bool isFloatOutput = (player->waveFormat.wBitsPerSample == 32);
                float vol = player->volume.load();
                UINT32 srcChannels = player->sourceFormat.nChannels;
                UINT32 dstChannels = player->waveFormat.nChannels;
                const short* allSrcSamples = reinterpret_cast<const short*>(player->audioData.data());
                
                if (isFloatOutput) {
                    float* dstData = reinterpret_cast<float*>(pData);
                    for (UINT32 f = 0; f < framesToRender; f++) {
                        double exactSrcPos = (player->currentPosition + (f * resampleRatio));
                        UINT32 srcIdx0 = (UINT32)exactSrcPos;
                        UINT32 srcIdx1 = min(srcIdx0 + 1, player->totalSamples - 1);
                        float frac = (float)(exactSrcPos - srcIdx0);
                        
                        for (UINT32 ch = 0; ch < dstChannels; ch++) {
                            UINT32 sCh = (ch < srcChannels) ? ch : (srcChannels - 1);
                            float s0 = allSrcSamples[srcIdx0 * srcChannels + sCh] / 32768.0f;
                            float s1 = allSrcSamples[srcIdx1 * srcChannels + sCh] / 32768.0f;
                            float sample = (s0 + frac * (s1 - s0)) * vol;
                            dstData[f * dstChannels + ch] = sample;
                        }
                    }
                } else {
                    short* dstData = reinterpret_cast<short*>(pData);
                    for (UINT32 f = 0; f < framesToRender; f++) {
                        double exactSrcPos = (player->currentPosition + (f * resampleRatio));
                        UINT32 srcIdx0 = (UINT32)exactSrcPos;
                        UINT32 srcIdx1 = min(srcIdx0 + 1, player->totalSamples - 1);
                        float frac = (float)(exactSrcPos - srcIdx0);
                        
                        for (UINT32 ch = 0; ch < dstChannels; ch++) {
                            UINT32 sCh = (ch < srcChannels) ? ch : (srcChannels - 1);
                            float s0 = allSrcSamples[srcIdx0 * srcChannels + sCh];
                            float s1 = allSrcSamples[srcIdx1 * srcChannels + sCh];
                            float sample = (s0 + frac * (s1 - s0)) * vol;
                            dstData[f * dstChannels + ch] = static_cast<short>(sample);
                        }
                    }
                }
                
                player->currentPosition += (UINT32)(framesToRender * resampleRatio);
                if (framesToRender == maxDstFramesCanFeed) {
                    player->currentPosition = player->totalSamples;
                }
                
                // Release only the frames actually rendered so WASAPI never buffers trailing silence
                hr = player->renderClient->ReleaseBuffer(framesToRender, flags);
                if (FAILED(hr)) break;
            } else {
                hr = player->renderClient->ReleaseBuffer(0, flags);
                player->currentPosition = player->totalSamples;
            }
        } else {
            Sleep(1);
        }
    }
    
    // Stop audio client
    if (player->audioClient) {
        player->audioClient->Stop();
    }
}

extern "C" {

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_createPlayer(JNIEnv* env, jclass clazz) {
    AudioPlayer* player = new AudioPlayer();
    
    // Initialize COM - S_FALSE is also OK (already initialized)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        delete player;
        return 0;
    }
    
    // Create device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&player->deviceEnumerator
    );
    
    if (FAILED(hr)) {
        CoUninitialize();
        delete player;
        return 0;
    }
    
    return reinterpret_cast<jlong>(player);
}

JNIEXPORT void JNICALL Java_fastaudio_FastAudioPlayer_destroyPlayer(JNIEnv* env, jclass clazz, jlong handle) {
    if (handle) {
        AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
        delete player;
        CoUninitialize();
    }
}

static bool InitAudioClient(AudioPlayer* player) {
    if (!player->deviceEnumerator) return false;

    // Release any previous audio client resources if reloaded
    if (player->renderClient) {
        player->renderClient->Release();
        player->renderClient = nullptr;
    }
    if (player->audioClient) {
        player->audioClient->Release();
        player->audioClient = nullptr;
    }
    if (player->audioDevice) {
        player->audioDevice->Release();
        player->audioDevice = nullptr;
    }

    HRESULT hr = player->deviceEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &player->audioDevice
    );
    if (FAILED(hr)) return false;
    
    hr = player->audioDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&player->audioClient
    );
    if (FAILED(hr)) return false;
    
    WAVEFORMATEX* mixFormat = nullptr;
    hr = player->audioClient->GetMixFormat(&mixFormat);
    if (FAILED(hr) || !mixFormat) {
        return false;
    }
    player->waveFormat = *mixFormat;
    
    REFERENCE_TIME bufferDuration = 10000000; // 1 second
    hr = player->audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        mixFormat,
        nullptr
    );
    CoTaskMemFree(mixFormat);
    
    if (FAILED(hr)) {
        return false;
    }
    
    hr = player->audioClient->GetBufferSize(&player->bufferFrameCount);
    if (FAILED(hr)) return false;
    
    hr = player->audioClient->GetService(
        __uuidof(IAudioRenderClient), (void**)&player->renderClient
    );
    if (FAILED(hr)) return false;
    
    return true;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_loadFile(JNIEnv* env, jclass clazz, jlong handle, jstring filePath) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    std::wstring path = UTF8ToWString(env, filePath);
    if (path.empty()) return JNI_FALSE;
    
    if (!ParseWavFile(path.c_str(), player->audioData, player->sourceFormat)) {
        return JNI_FALSE;
    }
    
    player->waveFormat = player->sourceFormat;
    player->totalSamples = player->audioData.size() / player->sourceFormat.nBlockAlign;
    player->currentPosition = 0;
    
    return InitAudioClient(player) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_loadMemory(JNIEnv* env, jclass clazz, jlong handle, jbyteArray data) {
    if (!handle || !data) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    jsize len = env->GetArrayLength(data);
    if (len < 44) return JNI_FALSE;
    
    jbyte* bytes = env->GetByteArrayElements(data, nullptr);
    if (!bytes) return JNI_FALSE;
    
    bool parsed = ParseWavMemory(reinterpret_cast<const BYTE*>(bytes), static_cast<size_t>(len), player->audioData, player->sourceFormat);
    env->ReleaseByteArrayElements(data, bytes, JNI_ABORT);
    
    if (!parsed) return JNI_FALSE;
    
    player->waveFormat = player->sourceFormat;
    player->totalSamples = player->audioData.size() / player->sourceFormat.nBlockAlign;
    player->currentPosition = 0;
    
    return InitAudioClient(player) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_play(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (player->isPlaying) return JNI_TRUE;
    if (player->audioData.empty()) return JNI_FALSE;
    
    player->isPlaying = true;
    player->isPaused = false;
    
    // Start playback thread
    player->playbackThread = std::thread(PlaybackThread, player);
    
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_pause(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (!player->isPlaying) return JNI_FALSE;
    
    player->isPaused = !player->isPaused;
    
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_resume(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (!player->isPlaying) return JNI_FALSE;
    
    player->isPaused = false;
    
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_stop(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    player->stopPlayback();
    player->currentPosition = 0;
    
    return JNI_TRUE;
}

JNIEXPORT void JNICALL Java_fastaudio_FastAudioPlayer_setVolume(JNIEnv* env, jclass clazz, jlong handle, jfloat volume) {
    if (!handle) return;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    player->volume = volume;
}

JNIEXPORT jfloat JNICALL Java_fastaudio_FastAudioPlayer_getVolume(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return 1.0f;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    return player->volume.load();
}

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_getPosition(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return 0;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (player->sourceFormat.nSamplesPerSec == 0) return 0;
    
    // Convert source frames to milliseconds
    return (player->currentPosition * 1000) / player->sourceFormat.nSamplesPerSec;
}

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_getDuration(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return 0;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (player->sourceFormat.nSamplesPerSec == 0) return 0;
    
    // Convert source total samples to milliseconds
    return (player->totalSamples * 1000) / player->sourceFormat.nSamplesPerSec;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_isPlaying(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    return player->isPlaying ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jobjectArray JNICALL Java_fastaudio_FastAudioPlayer_nativeGetDevices(JNIEnv* env, jclass clazz) {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    
    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDeviceCollection* devices = nullptr;
    
    HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&enumerator
    );
    
    if (FAILED(hr)) {
        CoUninitialize();
        jobjectArray result = env->NewObjectArray(1, env->FindClass("java/lang/String"), nullptr);
        jstring def = env->NewStringUTF("Default");
        env->SetObjectArrayElement(result, 0, def);
        return result;
    }
    
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &devices);
    if (FAILED(hr)) {
        enumerator->Release();
        CoUninitialize();
        jobjectArray result = env->NewObjectArray(1, env->FindClass("java/lang/String"), nullptr);
        jstring def = env->NewStringUTF("Default");
        env->SetObjectArrayElement(result, 0, def);
        return result;
    }
    
    UINT count;
    devices->GetCount(&count);
    
    std::vector<std::wstring> deviceNames;
    for (UINT i = 0; i < count; i++) {
        IMMDevice* device = nullptr;
        if (SUCCEEDED(devices->Item(i, &device))) {
            IPropertyStore* props = nullptr;
            if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props))) {
                PROPVARIANT varName;
                PropVariantInit(&varName);
                if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &varName))) {
                    deviceNames.push_back(varName.pwszVal);
                    PropVariantClear(&varName);
                }
                props->Release();
            }
            device->Release();
        }
    }
    
    devices->Release();
    enumerator->Release();
    CoUninitialize();
    
    // Create result array
    jint size = deviceNames.empty() ? 1 : static_cast<jint>(deviceNames.size());
    jobjectArray result = env->NewObjectArray(size, env->FindClass("java/lang/String"), nullptr);
    
    if (deviceNames.empty()) {
        jstring def = env->NewStringUTF("Default");
        env->SetObjectArrayElement(result, 0, def);
    } else {
        for (jint i = 0; i < size; i++) {
            jstring name = WStringToJString(env, deviceNames[i]);
            env->SetObjectArrayElement(result, i, name);
        }
    }
    
    return result;
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_setDevice(JNIEnv* env, jclass clazz, jlong handle, jstring deviceId) {
    // Device switching would require re-initialization
    // For now, just return true
    return JNI_TRUE;
}

} // extern "C"
