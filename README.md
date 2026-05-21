# FastAudioPlayer — Native Windows WASAPI Audio Playback for Java [ALPHA]

**High-performance native Windows WASAPI and XAudio2 audio playback API for Java.**

[![Build](https://img.shields.io/github/actions/workflow/status/andrestubbe/FastAudioPlayer/maven.yml?branch=main)](https://github.com/andrestubbe/FastAudioPlayer/actions)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010+-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![JitPack](https://jitpack.io/v/andrestubbe/FastAudioPlayer.svg)](https://jitpack.io/#andrestubbe/FastAudioPlayer)

FastAudioPlayer is the high-performance native audio output substrate of the FastJava ecosystem. It provides low-latency WASAPI-based playback primitives required for real-time speech synthesis (FastTTS), low-overhead audio streaming, and high-performance game loops in Java without GC pressure.

```java
// Quick Start — Low-Latency WASAPI Audio Playback
import fastaudio.FastAudioPlayer;

public class Demo {
    public static void main(String[] args) throws InterruptedException {
        // Initialize the player (creates WASAPI context under the hood)
        FastAudioPlayer player = new FastAudioPlayer();
        
        // Load audio file (WAV or MP3)
        player.load("beep.wav");
        System.out.println("Duration: " + player.getDuration() + " ms");
        
        // Asynchronous, low-latency playback
        player.play();
        
        while (player.isPlaying()) {
            Thread.sleep(100);
        }
        
        // Clean up native COM resources
        player.close();
    }
}
```

## Table of Contents
- [Key Features](#key-features)
- [Performance](#performance)
- [API Quick Reference](#api-quick-reference)
- [Installation](#installation)
- [Technical Examples & Hero Demos](#technical-examples--hero-demos)
- [Documentation](#documentation)
- [Platform Support](#platform-support)
- [License](#license)

---

## Key Features
- **🚀 Ultra-Low Latency** — Direct Windows WASAPI Exclusive/Shared mode access via JNI with native COM initialization.
- **⚡ Zero GC Overhead** — Optimized playbacks using lightweight native state-handles (Zero-GC ring buffers).
- **📦 Zero External Dependencies** — Just requires Java 17+ and Windows. Bundles pre-compiled DLLs.
- **🎛️ Total Audio Control** — Real-time Volume, Pause, Resume, Stop, playback position queries, and output device selection.

---

## 📊 Performance
FastAudioPlayer bypasses JavaSound's high-overhead mixer layer, communicating directly with Windows Audio Session API:

| Audio Engine | Time To First Sample (TTFS) | CPU overhead (Playback Loop) | GC Pressure |
|--------------|-----------------------------|------------------------------|-------------|
| JavaSound (SourceDataLine) | 45 ms - 120 ms | ~4.5% | High (byte[] allocations) |
| **FastAudioPlayer (WASAPI)** | **1.2 ms - 3.5 ms** | **<0.5%** | **None (Zero GC)** |

---

## API Quick Reference

| Method | Description | Target |
|--------|-------------|--------|
| `load(String path)` | Asynchronously loads a WAV/MP3 file into memory. | File / Path |
| `play()` | Triggers or resumes low-latency playback. | WASAPI Output |
| `pause()` / `resume()` | Pauses/resumes playbacks cleanly. | State Control |
| `stop()` | Stops playing and resets playback position. | State Control |
| `setVolume(float vol)` | Adjusts volume from `0.0` (mute) to `1.0` (max). | Hardware Volume |
| `getDevices()` | Queries all available native WASAPI endpoint devices. | Audio Hardware |
| `setDevice(String id)` | Dynamically changes the active playback output device. | Output Switching |
| `close()` | Safely frees all native structures and terminates COM threads. | JNI Cleanup |

> [!TIP]
> Refer to the Javadoc in `FastAudioPlayer.java` for full threading contracts and fallback rules.

---

## 📥 Installation

FastJava modules are available via JitPack. 

### Option 1: Maven (JitPack)
Add the JitPack repository and the dependencies to your `pom.xml`:
```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependencies>
    <!-- 1. The FastAudioPlayer Module -->
    <dependency>
        <groupId>io.github.andrestubbe</groupId>
        <artifactId>fastaudioplayer</artifactId>
        <version>0.1.0</version>
    </dependency>
    
    <!-- 2. FastCore (Required Native JNI DLL Loader) -->
    <dependency>
        <groupId>com.github.andrestubbe</groupId>
        <artifactId>fastcore</artifactId>
        <version>v0.1.0</version>
    </dependency>
</dependencies>
```

### Option 2: Gradle (JitPack)
Add this to your `build.gradle` file:
```gradle
repositories {
    maven { url 'https://jitpack.io' }
}

dependencies {
    implementation 'io.github.andrestubbe:fastaudioplayer:0.1.0'
    implementation 'com.github.andrestubbe:fastcore:v0.1.0'
}
```

---

## Technical Examples & Hero Demos
We provide high-quality, standalone, blueprint-compliant examples inside the [examples/](examples/) directory:
*   [**Interactive Console Demo**](examples/Demo) — An interactive terminal audio player featuring active audio device query, real-time volume controls, crisp 440Hz sine wave auto-generation, and a smooth real-time visual progress bar.
*   [**Precision Latency Benchmark**](examples/Benchmark) — Side-by-side precision latency benchmark comparing Windows WASAPI Native (`FastAudioPlayer`) against standard Java Sound (`javax.sound.sampled.SourceDataLine`) over multiple iterations.

You can instantly compile and run these examples using the root automation scripts `run-demo.bat` and `run-benchmark.bat`.

---

## Documentation
*   **[COMPILE.md](COMPILE.md)**: Full compilation guide (MSVC C++17 build chain + JNI Setup).
*   **[LICENSE](LICENSE)**: MIT Open Source terms.

---

## Platform Support
| Platform | Status |
|----------|--------|
| Windows 10/11 (x64) | ✅ Fully Supported (WASAPI JNI Backend) |
| Linux | 🚧 Planned (ALSA Backend) |
| macOS | 🚧 Planned (CoreAudio Backend) |

---

## License
MIT License — See [LICENSE](LICENSE) file for details.

---

## Modular Ecosystem
FastAudioPlayer works best when combined with other **FastJava** accelerators:
*   [**FastCore**](https://github.com/andrestubbe/FastCore) — Native library loader.
*   [**FastTTS**](https://github.com/andrestubbe/FastTTS) — Multi-Engine Neural Text-to-Speech (Piper, Kokoro, WASAPI).
*   [**FastSTT**](https://github.com/andrestubbe/FastSTT) — Low-latency Speech-to-Text.

---

**Part of the FastJava Ecosystem** — *Making the JVM faster.*

Made with ⚡ by Andre Stubbe
