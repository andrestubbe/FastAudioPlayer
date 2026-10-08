# FastAudioPlayer Changelog

## [0.1.2] - 2026-08-14
- Added in-memory direct buffer playback `load(byte[] data)` avoiding temporary disk WAV files and I/O latency.
- Added linear interpolation resampling from source rate (e.g. 16 kHz / 22.05 kHz) to hardware WASAPI mix format (48 kHz) in shared mode.
- Fixed `getDuration()` and `getPosition()` to calculate timestamps based on `sourceFormat.nSamplesPerSec` to eliminate premature audio cutoff.
- Added 1.18 Billion ops/s JMH Benchmark.
- Integrated FastSIMD AVX2 volume gain engine.
- Expanded full API Reference manual.

## [0.1.1] - 2026-06-14
- Initial release with WASAPI low-latency playback.
