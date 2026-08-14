package fastaudioplayer.benchmark;
import fastaudioplayer.FastAudioPlayer;
import org.openjdk.jmh.annotations.*;
import java.util.concurrent.TimeUnit;

@State(Scope.Thread)
@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
public class JMH_Player {
    private FastAudioPlayer player;
    @Setup public void setup() { player = new FastAudioPlayer(); }
    @Benchmark public FastAudioPlayer benchmarkPlayer() { return player; }
}