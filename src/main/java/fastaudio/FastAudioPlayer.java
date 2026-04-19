package fastaudio;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;

/**
 * FastAudioPlayer - Native low-latency audio playback for Java.
 * 
 * <p>Uses Windows WASAPI for direct hardware access with minimal latency.
 * Supports WAV and MP3 formats.</p>
 * 
 * <p><b>Features:</b></p>
 * <ul>
 *   <li>Play/Pause/Resume/Stop</li>
 *   <li>Volume control (0.0 - 1.0)</li>
 *   <li>Playback position</li>
 *   <li>Device selection</li>
 * </ul>
 * 
 * @author FastJava Team
 * @version 1.0.0
 */
public class FastAudioPlayer {
    
    private static final String LIBRARY_NAME = "fastaudioplayer";
    private long nativeHandle;
    
    static {
        loadNativeLibrary();
    }
    
    private static void loadNativeLibrary() {
        try {
            System.loadLibrary(LIBRARY_NAME);
        } catch (UnsatisfiedLinkError e) {
            try {
                String libResource = "/" + LIBRARY_NAME + ".dll";
                try (InputStream in = FastAudioPlayer.class.getResourceAsStream(libResource)) {
                    if (in == null) {
                        throw new RuntimeException("Native library not found: " + libResource);
                    }
                    Path tempDir = Files.createTempDirectory("fastaudio");
                    Path tempLib = tempDir.resolve(LIBRARY_NAME + ".dll");
                    Files.copy(in, tempLib, StandardCopyOption.REPLACE_EXISTING);
                    tempLib.toFile().deleteOnExit();
                    tempDir.toFile().deleteOnExit();
                    System.load(tempLib.toString());
                }
            } catch (Exception ex) {
                throw new RuntimeException("Failed to load native library", ex);
            }
        }
    }
    
    // Native methods
    private static native long createPlayer();
    private static native void destroyPlayer(long handle);
    private static native boolean loadFile(long handle, String filePath);
    private static native boolean play(long handle);
    private static native boolean pause(long handle);
    private static native boolean resume(long handle);
    private static native boolean stop(long handle);
    private static native void setVolume(long handle, float volume);
    private static native float getVolume(long handle);
    private static native long getPosition(long handle);
    private static native long getDuration(long handle);
    private static native boolean isPlaying(long handle);
    private static native String[] nativeGetDevices();
    private static native boolean setDevice(long handle, String deviceId);
    
    /**
     * Create a new audio player.
     */
    public FastAudioPlayer() {
        this.nativeHandle = createPlayer();
        if (this.nativeHandle == 0) {
            throw new RuntimeException("Failed to create audio player");
        }
    }
    
    /**
     * Load audio file (WAV or MP3).
     * @param filePath Path to audio file
     * @return true if loaded successfully
     */
    public boolean load(String filePath) {
        checkHandle();
        return loadFile(nativeHandle, filePath);
    }
    
    /**
     * Load audio file from File object.
     * @param file Audio file
     * @return true if loaded successfully
     */
    public boolean load(File file) {
        return load(file.getAbsolutePath());
    }
    
    /**
     * Start or resume playback.
     * @return true if successful
     */
    public boolean play() {
        checkHandle();
        return play(nativeHandle);
    }
    
    /**
     * Pause playback.
     * @return true if successful
     */
    public boolean pause() {
        checkHandle();
        return pause(nativeHandle);
    }
    
    /**
     * Resume from pause.
     * @return true if successful
     */
    public boolean resume() {
        checkHandle();
        return resume(nativeHandle);
    }
    
    /**
     * Stop playback and reset position.
     * @return true if successful
     */
    public boolean stop() {
        checkHandle();
        return stop(nativeHandle);
    }
    
    /**
     * Set volume (0.0 = mute, 1.0 = max).
     * @param Volume level
     */
    public void setVolume(float volume) {
        checkHandle();
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;
        setVolume(nativeHandle, volume);
    }
    
    /**
     * Get current volume.
     * @return Volume level (0.0 - 1.0)
     */
    public float getVolume() {
        checkHandle();
        return getVolume(nativeHandle);
    }
    
    /**
     * Get current playback position in milliseconds.
     * @return Position in ms
     */
    public long getPosition() {
        checkHandle();
        return getPosition(nativeHandle);
    }
    
    /**
     * Get total duration in milliseconds.
     * @return Duration in ms
     */
    public long getDuration() {
        checkHandle();
        return getDuration(nativeHandle);
    }
    
    /**
     * Check if currently playing.
     * @return true if playing
     */
    public boolean isPlaying() {
        checkHandle();
        return isPlaying(nativeHandle);
    }
    
    /**
     * Get list of available audio output devices.
     * @return Array of device IDs
     */
    public static String[] getDevices() {
        return nativeGetDevices();
    }
    
    /**
     * Select output device.
     * @param deviceId Device ID from getDevices()
     * @return true if successful
     */
    public boolean setDevice(String deviceId) {
        checkHandle();
        return setDevice(nativeHandle, deviceId);
    }
    
    /**
     * Close player and release resources.
     */
    public void close() {
        if (nativeHandle != 0) {
            destroyPlayer(nativeHandle);
            nativeHandle = 0;
        }
    }
    
    private void checkHandle() {
        if (nativeHandle == 0) {
            throw new IllegalStateException("Player is closed");
        }
    }
    
}
