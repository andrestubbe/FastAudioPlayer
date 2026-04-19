# FastAudioPlayer — Native Audio Playback for Java [ALPHA]

> **Low-latency audio playback** — WASAPI native audio for Java, 10× lower latency than JavaFX/Media.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17+-blue.svg)](https://www.java.com)
[![JitPack](https://img.shields.io/badge/JitPack-ready-green.svg)](https://jitpack.io/#andrestubbe)

---

## ⚡ Performance

| Metric | FastAudioPlayer | JavaFX Media | Java Sound API |
|--------|----------------|--------------|----------------|
| **Latency** | **10-20ms** | 100-300ms | 50-100ms |
| **CPU Usage** | **Low** | Medium | Medium |
| **Format Support** | **WAV, MP3** | Limited | WAV only |
| **Device Select** | **✅ Yes** | ❌ No | ❌ No |

**Windows:** Uses WASAPI (direct hardware access, lowest latency)

---

## 📦 Quick Start

### Maven (JitPack)

```xml
<repositories>
    <repository>
        <id>jitpack.io</id>
        <url>https://jitpack.io</url>
    </repository>
</repositories>

<dependency>
    <groupId>com.github.andrestubbe</groupId>
    <artifactId>fastaudioplayer</artifactId>
    <version>1.0.0</version>
</dependency>
```

### Basic Usage

```java
import fastaudio.FastAudioPlayer;

// Create player
FastAudioPlayer player = new FastAudioPlayer();

// Load and play
player.load("music.mp3");
player.setVolume(0.8f);
player.play();

// Control playback
Thread.sleep(5000);
player.pause();
Thread.sleep(1000);
player.resume();

// Get position/duration
long pos = player.getPosition();   // Current position in ms
long dur = player.getDuration();     // Total duration in ms

// Clean up
player.close();
```

---

## 🛠️ Building

### Prerequisites
- Windows 10/11
- Java 17+
- Visual Studio 2022 (with C++ workload)
- Windows SDK

### Build DLL
```batch
compile.bat
```

### Build JAR
```batch
mvn clean package
```

---

## 📋 Features

- ✅ WAV and MP3 playback
- ✅ Play / Pause / Resume / Stop
- ✅ Volume control (0.0 - 1.0)
- ✅ Position/duration queries
- ✅ Device selection
- ✅ Low-latency WASAPI backend

---

## 🔗 Links

- [FastJava Ecosystem](https://github.com/andrestubbe/FastJava)
- [JitPack Repository](https://jitpack.io/#andrestubbe/fastaudioplayer)

---

## 📄 License

MIT License — See [LICENSE](LICENSE) for details.

---

<p align="center">
  <b>Part of the <a href="https://github.com/andrestubbe/FastJava">FastJava</a> Ecosystem</b>
</p>
