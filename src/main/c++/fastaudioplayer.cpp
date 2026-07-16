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

// Parse WAV file header
bool ParseWavFile(const wchar_t* filePath, std::vector<BYTE>& audioData, WAVEFORMATEX& format) {
    HANDLE hFile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    // Read RIFF header
    BYTE header[44];
    DWORD bytesRead;
    if (!ReadFile(hFile, header, 44, &bytesRead, nullptr) || bytesRead != 44) {
        CloseHandle(hFile);
        return false;
    }
    
    // Check "RIFF" and "WAVE" signatures
    if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
        CloseHandle(hFile);
        return false;
    }
    
    
    // Parse format
    format.wFormatTag = *(WORD*)(header + 20);
    format.nChannels = *(WORD*)(header + 22);
    format.nSamplesPerSec = *(DWORD*)(header + 24);
    format.nAvgBytesPerSec = *(DWORD*)(header + 28);
    format.nBlockAlign = *(WORD*)(header + 32);
    format.wBitsPerSample = *(WORD*)(header + 34);
    format.cbSize = 0;
    
    // Check for extended format (WAVEFORMATEXTENSIBLE)
    if (format.wFormatTag == 0xFFFE && format.cbSize >= 22) {
        // Extended header - read SubFormat GUID at offset 24
        WORD extSize = *(WORD*)(header + 36);
        WORD validBits = *(WORD*)(header + 38);
        DWORD channelMask = *(DWORD*)(header + 40);
        // SubFormat GUID at offset 44
        WORD subFormatLow = *(WORD*)(header + 44);
        // If subformat indicates PCM (1), use PCM
        if (subFormatLow == 1) {
            format.wFormatTag = WAVE_FORMAT_PCM;
        }
    }
    
    // Calculate actual position after fmt chunk
    // Standard WAV: fmt chunk is at offset 12, size is at offset 16
    DWORD fmtChunkSize = *(DWORD*)(header + 16);
    DWORD pos = 12 + 8 + fmtChunkSize;  // 12 ("RIFF"+size+"WAVE") + 8 ("fmt "+size) + fmtChunkSize
    // Align to word boundary
    if (fmtChunkSize % 2) pos++;
    
    // Find "data" chunk
    DWORD fileSize = *(DWORD*)(header + 4) + 8;
    BYTE chunkHeader[8];
    DWORD dataSize = 0;
    
    while (pos < fileSize) {
        SetFilePointer(hFile, pos, nullptr, FILE_BEGIN);
        if (!ReadFile(hFile, chunkHeader, 8, &bytesRead, nullptr) || bytesRead != 8) {
            break;
        }
        
        DWORD chunkSize = *(DWORD*)(chunkHeader + 4);
        char chunkName[5] = {0};
        memcpy(chunkName, chunkHeader, 4);
        
        if (memcmp(chunkHeader, "data", 4) == 0) {
            dataSize = chunkSize;
            audioData.resize(dataSize);
            
            if (ReadFile(hFile, audioData.data(), dataSize, &bytesRead, nullptr)) {
                audioData.resize(bytesRead);
            }
            break;
        }
        
        pos += 8 + chunkSize;
        if (chunkSize % 2) pos++; // Pad byte
    }
    
    CloseHandle(hFile);
    return !audioData.empty();
}

// Playback thread function
void PlaybackThread(AudioPlayer* player) {
    HRESULT hr;
    UINT32 numFramesAvailable;
    BYTE* pData;
    DWORD flags = 0;
    
    fprintf(stderr, "[Playback] Thread started\n");
    fprintf(stderr, "[Playback] Source: %dHz %dbit %dch, align=%d\n", 
            player->sourceFormat.nSamplesPerSec, player->sourceFormat.wBitsPerSample, 
            player->sourceFormat.nChannels, player->sourceFormat.nBlockAlign);
    fprintf(stderr, "[Playback] Output: %dHz %dbit %dch, align=%d\n", 
            player->waveFormat.nSamplesPerSec, player->waveFormat.wBitsPerSample, 
            player->waveFormat.nChannels, player->waveFormat.nBlockAlign);
    
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
        
        // Get buffer size
        UINT32 currentPadding;
        hr = player->audioClient->GetCurrentPadding(&currentPadding);
        if (FAILED(hr)) break;
        
        numFramesAvailable = player->bufferFrameCount - currentPadding;
        
        if (numFramesAvailable > 0) {
            // Get buffer
            hr = player->renderClient->GetBuffer(numFramesAvailable, &pData);
            if (FAILED(hr)) {
                fprintf(stderr, "[Playback] GetBuffer failed: 0x%08X\n", hr);
                break;
            }
            if (!pData) {
                fprintf(stderr, "[Playback] pData is NULL!\n");
                break;
            }
            
            // Calculate buffer sizes
            UINT32 dstBytesPerFrame = player->waveFormat.nBlockAlign;  // Output format (could be 32-bit float)
            UINT32 srcBytesPerFrame = player->sourceFormat.nBlockAlign; // Source format (16-bit PCM)
            UINT32 bytesAvailable = numFramesAvailable * dstBytesPerFrame;
            UINT32 remainingFrames = player->totalSamples - player->currentPosition;
            UINT32 framesToCopy = min(numFramesAvailable, remainingFrames);
            
            if (framesToCopy > 0) {
                fprintf(stderr, "[Playback] Copying %u frames (pos=%u/%u)\n", framesToCopy, player->currentPosition, player->totalSamples);
                
                // Check if we need format conversion
                bool needsConversion = (player->waveFormat.wBitsPerSample != player->sourceFormat.wBitsPerSample) ||
                                       (player->waveFormat.nSamplesPerSec != player->sourceFormat.nSamplesPerSec);
                
                if (needsConversion && player->sourceFormat.wBitsPerSample == 16 && player->waveFormat.wBitsPerSample == 32) {
                    fprintf(stderr, "[Playback] Converting 16-bit to 32-bit float\n");
                    // Convert 16-bit PCM to 32-bit float
                    const short* srcData = reinterpret_cast<const short*>(
                        player->audioData.data() + (player->currentPosition * srcBytesPerFrame));
                    float* dstData = reinterpret_cast<float*>(pData);
                    UINT32 numSrcSamples = framesToCopy * player->sourceFormat.nChannels;
                    
                    for (UINT32 i = 0; i < numSrcSamples; i++) {
                        float sample = srcData[i] / 32768.0f * player->volume.load();
                        // Duplicate for destination channels if needed
                        dstData[i] = sample;
                    }
                } else {
                    // Same format - direct copy
                    UINT32 bytesToCopy = framesToCopy * srcBytesPerFrame;
                    memcpy(pData, player->audioData.data() + (player->currentPosition * srcBytesPerFrame), bytesToCopy);
                    
                    // Apply volume for 16-bit
                    if (player->volume < 1.0f && player->waveFormat.wBitsPerSample == 16) {
                        short* samples = reinterpret_cast<short*>(pData);
                        UINT32 numSamples = (bytesToCopy / 2);
                        for (UINT32 i = 0; i < numSamples; i++) {
                            samples[i] = static_cast<short>(samples[i] * player->volume.load());
                        }
                    }
                }
                
                // Zero remaining buffer
                UINT32 copiedBytes = framesToCopy * dstBytesPerFrame;
                if (copiedBytes < bytesAvailable) {
                    memset(reinterpret_cast<BYTE*>(pData) + copiedBytes, 0, bytesAvailable - copiedBytes);
                }
                
                player->currentPosition += framesToCopy;
            } else {
                // End of file - write silence
                memset(pData, 0, bytesAvailable);
                player->isPlaying = false;
            }
            
            // Release buffer
            hr = player->renderClient->ReleaseBuffer(numFramesAvailable, flags);
            if (FAILED(hr)) break;
        } else {
            Sleep(1);
        }
        
        // Check if finished
        if (player->currentPosition >= player->totalSamples) {
            player->isPlaying = false;
        }
    }
    
    // Stop audio client
    fprintf(stderr, "[Playback] Stopping audio client...\n");
    if (player->audioClient) {
        player->audioClient->Stop();
    }
    fprintf(stderr, "[Playback] Thread finished\n");
}

extern "C" {

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_createPlayer(JNIEnv* env, jclass clazz) {
    fprintf(stderr, "[createPlayer] Creating player...\n");
    AudioPlayer* player = new AudioPlayer();
    
    // Initialize COM - S_FALSE is also OK (already initialized)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    fprintf(stderr, "[createPlayer] CoInitializeEx returned: 0x%08X\n", hr);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        fprintf(stderr, "[createPlayer] COM init failed!\n");
        delete player;
        return 0;
    }
    
    // Create device enumerator
    fprintf(stderr, "[createPlayer] Creating device enumerator...\n");
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**)&player->deviceEnumerator
    );
    
    fprintf(stderr, "[createPlayer] CoCreateInstance returned: 0x%08X, deviceEnumerator=%p\n", hr, player->deviceEnumerator);
    
    if (FAILED(hr)) {
        fprintf(stderr, "[createPlayer] Device enumerator creation failed!\n");
        CoUninitialize();
        delete player;
        return 0;
    }
    
    fprintf(stderr, "[createPlayer] Player created successfully, handle=%p\n", player);
    return reinterpret_cast<jlong>(player);
}

JNIEXPORT void JNICALL Java_fastaudio_FastAudioPlayer_destroyPlayer(JNIEnv* env, jclass clazz, jlong handle) {
    if (handle) {
        AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
        delete player;
        CoUninitialize();
    }
}

JNIEXPORT jboolean JNICALL Java_fastaudio_FastAudioPlayer_loadFile(JNIEnv* env, jclass clazz, jlong handle, jstring filePath) {
    fprintf(stderr, "[loadFile] Called with handle=%p (value=%lld)\n", reinterpret_cast<void*>(handle), handle);
    if (!handle) return JNI_FALSE;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    fprintf(stderr, "[loadFile] player=%p, deviceEnumerator=%p\n", player, player->deviceEnumerator);
    
    // Get file path
    std::wstring path = UTF8ToWString(env, filePath);
    if (path.empty()) return JNI_FALSE;
    
    // Parse WAV file
    if (!ParseWavFile(path.c_str(), player->audioData, player->sourceFormat)) {
        return JNI_FALSE;
    }
    
    // Store original format
    player->waveFormat = player->sourceFormat;
    
    player->totalSamples = player->audioData.size() / player->sourceFormat.nBlockAlign;
    player->currentPosition = 0;
    
    // Get default audio device
    fprintf(stderr, "[loadFile] deviceEnumerator=%p\n", player->deviceEnumerator);
    if (!player->deviceEnumerator) {
        fprintf(stderr, "[loadFile] ERROR: deviceEnumerator is NULL!\n");
        return JNI_FALSE;
    }
    fprintf(stderr, "[loadFile] Getting default audio endpoint...\n");
    HRESULT hr = player->deviceEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &player->audioDevice
    );
    if (FAILED(hr)) {
        fprintf(stderr, "[loadFile] GetDefaultAudioEndpoint failed: 0x%08X\n", hr);
        return JNI_FALSE;
    }
    
    // Activate audio client
    fprintf(stderr, "[loadFile] Activating audio client...\n");
    hr = player->audioDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&player->audioClient
    );
    if (FAILED(hr)) {
        fprintf(stderr, "[loadFile] Activate failed: 0x%08X\n", hr);
        return JNI_FALSE;
    }
    
    // Initialize audio client with WAV format (should work for 16-bit PCM)
    fprintf(stderr, "[loadFile] Initializing with WAV format...\n");
    fprintf(stderr, "[loadFile] WAV Format: %dHz, %dch, %dbits, align=%d\n",
            player->waveFormat.nSamplesPerSec, player->waveFormat.nChannels, 
            player->waveFormat.wBitsPerSample, player->waveFormat.nBlockAlign);
    
    REFERENCE_TIME bufferDuration = 10000000; // 1 second
    hr = player->audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        &player->waveFormat,
        nullptr
    );
    
    fprintf(stderr, "[loadFile] Initialize returned: 0x%08X\n", hr);
    
    if (FAILED(hr)) {
        fprintf(stderr, "[loadFile] Initialize failed, trying mix format...\n");
        // WAV format not supported, get mix format
        WAVEFORMATEX* mixFormat = nullptr;
        HRESULT hr2 = player->audioClient->GetMixFormat(&mixFormat);
        if (SUCCEEDED(hr2)) {
            // Try with mix format
            player->audioClient->Release();
            player->audioClient = nullptr;
            
            hr = player->audioDevice->Activate(
                __uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&player->audioClient
            );
            if (SUCCEEDED(hr)) {
                hr = player->audioClient->Initialize(
                    AUDCLNT_SHAREMODE_SHARED,
                    0,
                    bufferDuration,
                    0,
                    mixFormat,
                    nullptr
                );
                if (SUCCEEDED(hr)) {
                    player->waveFormat = *mixFormat;
                }
            }
            CoTaskMemFree(mixFormat);
        }
    }
    
    if (FAILED(hr)) {
        return JNI_FALSE;
    }
    
    // Get buffer size
    hr = player->audioClient->GetBufferSize(&player->bufferFrameCount);
    if (FAILED(hr)) return JNI_FALSE;
    
    // Get render client
    hr = player->audioClient->GetService(
        __uuidof(IAudioRenderClient), (void**)&player->renderClient
    );
    if (FAILED(hr)) return JNI_FALSE;
    
    return JNI_TRUE;
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
    
    if (player->waveFormat.nSamplesPerSec == 0) return 0;
    
    // Convert samples to milliseconds
    return (player->currentPosition * 1000) / player->waveFormat.nSamplesPerSec;
}

JNIEXPORT jlong JNICALL Java_fastaudio_FastAudioPlayer_getDuration(JNIEnv* env, jclass clazz, jlong handle) {
    if (!handle) return 0;
    
    AudioPlayer* player = reinterpret_cast<AudioPlayer*>(handle);
    
    if (player->waveFormat.nSamplesPerSec == 0) return 0;
    
    // Convert total samples to milliseconds
    return (player->totalSamples * 1000) / player->waveFormat.nSamplesPerSec;
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
