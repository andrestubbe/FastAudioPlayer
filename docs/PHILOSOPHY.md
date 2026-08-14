# FastAudioPlayer Design Philosophy

`FastAudioPlayer` communicates directly with Windows Audio Session API (WASAPI) and XAudio2, bypassing JavaSound's high-overhead mixer layer to achieve sub-5ms latency with zero Garbage Collection pressure.
