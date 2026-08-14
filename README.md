# FastAudioPlayer 0.1.2 [ALPHA-2026-08] — High-Performance Native Audio Playback for Java

[![Status](https://img.shields.io/badge/status-0.1.2-brightgreen.svg)](https://github.com/andrestubbe/FastAudioPlayer/releases/tag/0.1.2)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![JitPack](https://img.shields.io/badge/JitPack-0.1.2-green.svg)](https://jitpack.io/#andrestubbe/FastAudioPlayer)

**🔊 High-performance native Windows WASAPI and XAudio2 audio playback API for Java.**

FastAudioPlayer is the high-performance native audio output substrate of the FastJava ecosystem. It provides low-latency
WASAPI-based playback primitives required for real-time speech synthesis (FastTTS), low-overhead audio streaming, and
high-performance game loops in Java without GC pressure.

[![Showcase](https://raw.githubusercontent.com/andrestubbe/FastAudioPlayer/main/docs/screenshot.png)](https://www.youtube.com/watch?v=BZsqQl7WqWk)

---

## Quick Start — Example

```java
import fastaudio.FastAudioPlayer;

public class Demo {
    public static void main(String[] args) throws Exception {
        // Initialize player (creates WASAPI context)
        FastAudioPlayer player = new FastAudioPlayer();

        // Load audio file (WAV or MP3)
        player.load("beep.wav");
        System.out.println("Duration: " + player.getDuration() + " ms");

        // Asynchronous low-latency playback
        player.play();

        while (player.isPlaying()) {
            Thread.sleep(100);
        }

        // Clean up native resources
        player.close();
    }
}
```

## Table of Contents

- [Quick Start](#quick-start--example)
- [Key Features](#key-features)
- [Real-World Use Cases](#real-world-use-cases)
- [Performance Benchmarks](#performance-benchmarks)
- [API Quick Reference](#api-quick-reference)
- [Installation](#installation)
- [Technical Examples](#technical-examples)
- [Documentation](#documentation)
- [Platform Support](#platform-support)
- [License](#license)

---

## Key Features

- **⏱️ Ultra-Low Latency**: Direct Windows WASAPI Exclusive/Shared mode access via JNI with native COM initialization.
- **⚙️ Zero GC Overhead**: Optimized playbacks using lightweight native state-handles (Zero-GC ring buffers).
- **📦 Zero External Dependencies**: Just requires Java 17+ and Windows. Bundles pre-compiled DLLs.
- **🎛️ Total Audio Control**: Real-time Volume, Pause, Resume, Stop, playback position queries, and output device selection.

---

## Real-World Use Cases

- 🔊 **Low-Latency Game & GUI Audio**: Sub-10ms audio buffer playback via native WASAPI endpoints.
- 🎙️ **Voice AI Assistant Playback**: Stream synthetic speech directly from **[FastTTS](https://github.com/andrestubbe/FastTTS)** with zero buffer stutter.
- 🎚️ **Hardware Gain & Equalization**: Apply AVX2 SIMD volume scaling and pitch adjustments on off-heap PCM buffers.
- 🎵 **Multi-Track Audio Mixing**: Route multiple non-blocking native sound streams in real-time desktop applications.

---

## ⚡ Performance

FastAudioPlayer bypasses JavaSound's high-overhead mixer layer, communicating directly with Windows Audio Session API:

| Audio Engine                 | Time To First Sample (TTFS) | CPU overhead (Playback Loop) | GC Pressure               |
|------------------------------|-----------------------------|------------------------------|---------------------------|
| JavaSound (SourceDataLine)   | 45 ms - 120 ms              | ~4.5%                        | High (byte[] allocations) |
| **FastAudioPlayer (WASAPI)** | **1.2 ms - 3.5 ms**         | **<0.5%**                    | **None (Zero GC)**        |

---

## API Quick Reference

| Method                 | Description                                                    | Target           |
|------------------------|----------------------------------------------------------------|------------------|
| `load(String path)`    | Asynchronously loads a WAV/MP3 file into memory.               | File / Path      |
| `play()`               | Triggers or resumes low-latency playback.                      | WASAPI Output    |
| `pause()` / `resume()` | Pauses/resumes playbacks cleanly.                              | State Control    |
| `stop()`               | Stops playing and resets playback position.                    | State Control    |
| `setVolume(float vol)` | Adjusts volume from `0.0` (mute) to `1.0` (max).               | Hardware Volume  |
| `getDevices()`         | Queries all available native WASAPI endpoint devices.          | Audio Hardware   |
| `setDevice(String id)` | Dynamically changes the active playback output device.         | Output Switching |
| `close()`              | Safely frees all native structures and terminates COM threads. | JNI Cleanup      |

> [!TIP]
> Refer to the Javadoc in `FastAudioPlayer.java` for full threading contracts and fallback rules.

---

---

## Installation

### Option 1: Maven (Recommended)

Add the JitPack repository and the complete dependency stack to your `pom.xml`:

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- FastAudioPlayer Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastAudioPlayer</artifactId>
        <version>0.1.2</version>
    </dependency>

    <!-- FastSIMD Hardware Vector Acceleration Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastSIMD</artifactId>
        <version>0.1.3</version>
    </dependency>

    <!-- FastMemory Aligned Allocator -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastMemory</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastPointer Address Wrapper -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastPointer</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastAudioProcess Audio Engine -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastAudioProcess</artifactId>
        <version>0.1.1</version>
    </dependency>

    <!-- FastCore Unified JNI Loader -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>FastCore</artifactId>
        <version>0.1.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (via JitPack)

```groovy
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'com.github.andrestubbe:FastAudioPlayer:0.1.2'
    implementation 'com.github.andrestubbe:FastSIMD:0.1.3'
    implementation 'com.github.andrestubbe:FastMemory:0.1.1'
    implementation 'com.github.andrestubbe:FastPointer:0.1.1'
    implementation 'com.github.andrestubbe:FastAudioProcess:0.1.1'
    implementation 'com.github.andrestubbe:FastCore:0.1.0'
}
```

### Option 3: Direct Download (No Build Tool)

Download the latest JARs directly to add them to your classpath:

1. 📦 **[FastAudioPlayer-0.1.2.jar](https://github.com/andrestubbe/FastAudioPlayer/releases/download/0.1.2/FastAudioPlayer-0.1.2.jar)** (Native WASAPI Player)
2. ⚙️ **[fastcore-0.1.0.jar](https://github.com/andrestubbe/FastCore/releases/download/0.1.0/fastcore-0.1.0.jar)** (Required JNI Loader)

---

## Technical Examples

We provide high-quality, standalone examples inside the [examples/](examples/) directory:

* [**Interactive Console Demo**](examples/Demo)  An interactive terminal audio player featuring active audio device
  query, real-time volume controls, crisp 440Hz sine wave auto-generation, and a smooth real-time visual progress bar.
* [**Precision Latency Benchmark**](examples/Benchmark)  Side-by-side precision latency benchmark comparing Windows
  WASAPI Native (`FastAudioPlayer`) against standard Java Sound (`javax.sound.sampled.SourceDataLine`) over multiple
  iterations.

You can instantly compile and run these examples using the root automation scripts `run-demo.bat` and
`run-benchmark.bat`.

---

## Documentation

* **[CHANGELOG.md](docs/CHANGELOG.md)**: Release notes and version history.
* **[REFERENCE.md](docs/REFERENCE.md)**: Core API reference manual.
* **[PHILOSOPHY.md](docs/PHILOSOPHY.md)**: Engineering rationale for zero-allocation performance.
* **[COMPILE.md](docs/COMPILE.md)**: Full compilation guide (MSVC C++17 build chain + JNI Setup).
* **[ROADMAP.md](docs/ROADMAP.md)**: Future development goals.
---

## Platform Support

| Platform      | Status            |
|---------------|-------------------|
| Windows 10/11 | ✅ Fully Supported |
| Linux         | 🔗 Planned        |
| macOS         | 🔗 Planned        |

---

## License

MIT License  See [LICENSE](LICENSE) file for details.

---

## Related Projects

- [FastCore](https://github.com/andrestubbe/FastCore)  Native Library Loader for Java
- [FastAudioCapture](https://github.com/andrestubbe/FastAudioCapture)  High-Performance Native Audio Capture for Java
- [FastTTS](https://github.com/andrestubbe/FastTTS)  High-Performance Native Windows TTS API for Java
- [FastSTT](https://github.com/andrestubbe/FastSTT)  Ultra-Fast Native Speech-to-Text for Java
- [FastWakeWord](https://github.com/andrestubbe/FastWakeWord)

---

**Part of the FastJava Ecosystem** — *Making the JVM faster. Small package. Maximum speed. Zero bloat. 🚀📋*
