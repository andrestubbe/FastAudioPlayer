import fastaudio.FastAudioPlayer;
import javax.sound.sampled.*;
import java.io.File;

public class Benchmark {
    public static void main(String[] args) throws Exception {
        String wavFile = "beep.wav";
        int iterations = 5;
        
        System.out.println("=== AUDIO PLAYBACK BENCHMARK ===");
        System.out.println("File: " + wavFile);
        System.out.println("Iterations: " + iterations);
        System.out.println();
        
        // Test 1: Java Sound API (user's implementation reference)
        System.out.println("--- Test 1: Java Sound API (javax.sound.sampled) ---");
        long javaTotalTime = 0;
        for (int i = 0; i < iterations; i++) {
            long start = System.nanoTime();
            playWithJavaSound(wavFile);
            long end = System.nanoTime();
            long duration = (end - start) / 1_000_000; // ms
            javaTotalTime += duration;
            System.out.println("  Run " + (i+1) + ": " + duration + " ms");
            Thread.sleep(500);
        }
        System.out.println("  Average: " + (javaTotalTime / iterations) + " ms");
        System.out.println();
        
        // Test 2: FastAudioPlayer (WASAPI)
        System.out.println("--- Test 2: FastAudioPlayer (WASAPI) ---");
        long wasapiTotalTime = 0;
        boolean wasapiWorked = false;
        for (int i = 0; i < iterations; i++) {
            long start = System.nanoTime();
            wasapiWorked = playWithWasapi(wavFile);
            long end = System.nanoTime();
            long duration = (end - start) / 1_000_000; // ms
            wasapiTotalTime += duration;
            System.out.println("  Run " + (i+1) + ": " + duration + " ms (sound: " + wasapiWorked + ")");
            Thread.sleep(500);
        }
        System.out.println("  Average: " + (wasapiTotalTime / iterations) + " ms");
        System.out.println();
        
        // Summary
        System.out.println("=== SUMMARY ===");
        System.out.println("Java Sound API: " + (javaTotalTime / iterations) + " ms avg, SOUND: YES");
        System.out.println("WASAPI: " + (wasapiTotalTime / iterations) + " ms avg, SOUND: " + (wasapiWorked ? "YES" : "NO"));
    }
    
    static void playWithJavaSound(String filePath) throws Exception {
        File file = new File(filePath);
        AudioInputStream ais = AudioSystem.getAudioInputStream(file);
        AudioFormat format = ais.getFormat();
        
        DataLine.Info info = new DataLine.Info(SourceDataLine.class, format);
        SourceDataLine line = (SourceDataLine) AudioSystem.getLine(info);
        line.open(format);
        line.start();
        
        byte[] buffer = new byte[4096];
        int read;
        while ((read = ais.read(buffer)) > 0) {
            line.write(buffer, 0, read);
        }
        
        line.drain();
        line.stop();
        line.close();
        ais.close();
    }
    
    static boolean playWithWasapi(String filePath) throws Exception {
        FastAudioPlayer player = new FastAudioPlayer();
        boolean loaded = player.load(filePath);
        if (!loaded) {
            player.close();
            return false;
        }
        
        player.setVolume(1.0f);
        boolean playing = player.play();
        if (!playing) {
            player.close();
            return false;
        }
        
        // Wait for playback to finish
        while (player.isPlaying()) {
            Thread.sleep(10);
        }
        
        player.close();
        return true;
    }
}
