# FastAudioPlayer API Reference Manual

`FastAudioPlayer` provides native low-latency WASAPI audio playback for Java applications.

---

## 1. Core API

### `play` / `playBuffer`
```java
public void play(byte[] pcmData, int sampleRate)
```
Plays raw PCM audio data through low-latency WASAPI output endpoints.
