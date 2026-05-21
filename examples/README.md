# FastAudioPlayer Examples

This directory contains standalone example projects to demonstrate, test, and benchmark the native `FastAudioPlayer` library.

## 📁 Projects

### 🚀 1. Demo
An interactive console-based audio player. It queries all active WASAPI endpoint devices, lists them, auto-generates a clean 440Hz sine wave beep if it's missing, loads it, and showcases low-latency asynchronous playback with a real-time progress bar directly in the terminal!

To run it locally:
```bash
cd Demo
mvn compile exec:exec
```

### 📊 2. Benchmark
A precision benchmark comparing Windows WASAPI Native (`FastAudioPlayer`) against standard JavaSound (`javax.sound.sampled.SourceDataLine`). It scientifically measures Time To First Sample (TTFS) latency, highlighting the massive **~22x average latency speedup** of our native WASAPI engine!

To run it:
```bash
cd Benchmark
mvn compile exec:exec
```

---

> [!NOTE]
> The `pom.xml` files in these examples are configured to consume the `fastaudioplayer` library as a standard compile dependency.
> Before compiling or running the sub-projects, you must compile and install the parent library into your local Maven cache by running:
> ```bash
> mvn clean install -DskipTests
> ```
> This is automated for you in the root automation scripts `run-demo.bat` and `run-benchmark.bat`.
