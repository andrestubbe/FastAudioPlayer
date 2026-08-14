# FastAudioPlayer API Reference Manual

`FastAudioPlayer` provides native low-latency Windows WASAPI audio playback for Java applications with sub-5ms latency and zero JVM Garbage Collection pressure.

---

## 1. Class Construction & Resource Management

### `FastAudioPlayer()`
```java
public FastAudioPlayer()
```
Constructs a new native WASAPI audio player instance and initializes background COM sound threads.

---

### `close()`
```java
public void close()
```
Safely terminates WASAPI streaming threads, releases native COM buffers, and frees off-heap memory.

---

## 2. File Loading & Audio Playback API

### `load`
```java
public void load(String filePath) throws Exception
public void load(File file) throws Exception
```
Asynchronously decodes and loads WAV or MP3 audio files into direct off-heap PCM memory buffers.

---

### `play`
```java
public void play()
```
Begins or resumes asynchronous low-latency audio playback via native WASAPI endpoints.

---

### `pause`
```java
public void pause()
```
Pauses active audio playback while preserving current position pointers.

---

### `resume`
```java
public void resume()
```
Resumes paused audio playback instantly without buffer re-allocation.

---

### `stop`
```java
public void stop()
```
Stops playback immediately and resets position pointers to the beginning.

---

## 3. Volume & Device Controls

### `setVolume`
```java
public void setVolume(float volume)
```
Adjusts output volume from `0.0f` (mute) to `1.0f` (maximum hardware gain).

---

### `getDevices`
```java
public List<AudioDevice> getDevices()
```
Returns a list of all active native WASAPI output endpoint devices available on the system.

---

### `setDevice`
```java
public void setDevice(String deviceId)
```
Dynamically routes audio playback to a specific hardware output endpoint device by ID.

---

## 4. State Query API

| Method | Return Type | Description |
|:---|:---:|:---|
| `isPlaying()` | `boolean` | Returns `true` if audio is currently streaming through WASAPI. |
| `isPaused()` | `boolean` | Returns `true` if playback is currently paused. |
| `getDuration()` | `long` | Returns total audio duration in milliseconds. |
| `getPosition()` | `long` | Returns current playback position in milliseconds. |
