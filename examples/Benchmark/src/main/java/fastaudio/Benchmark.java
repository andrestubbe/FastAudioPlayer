package fastaudio;

import fastaudio.FastAudioPlayer;
import javax.sound.sampled.*;
import java.io.File;
import java.io.FileOutputStream;

/**
 * FastAudioPlayer Benchmark
 * 
 * Compares Time To First Sample (TTFS) latency and overhead between
 * standard JavaSound (javax.sound.sampled) and native WASAPI (FastAudioPlayer).
 */
public class Benchmark {
    
    public static void main(String[] args) throws Exception {
        String wavFile = args.length > 0 ? args[0] : "beep.wav";
        int iterations = args.length > 1 ? Integer.parseInt(args[1]) : 3;
        
        System.out.println("╔════════════════════════════════════════════════════════════╗");
        System.out.println("║     FastAudioPlayer - Console Latency Benchmark            ║");
        System.out.println("╚════════════════════════════════════════════════════════════╝");
        System.out.println();
        System.out.println("File: " + wavFile);
        System.out.println("Iterations: " + iterations);
        System.out.println();
        
        // Auto-create test file if not present
        if (!new File(wavFile).exists()) {
            System.out.println("🔊 '" + wavFile + "' not found. Generating a clean 440Hz sine wave...");
            generateBeepWav(wavFile);
            System.out.println("  ✓ Generated successfully.\n");
        }
        
        // Benchmark 1: Java Sound API
        System.out.println("┌────────────────────────────────────────────────────────┐");
        System.out.println("│  Test 1: Java Sound API (javax.sound.sampled)          │");
        System.out.println("└────────────────────────────────────────────────────────┘");
        long javaTime = benchmarkJavaSound(wavFile, iterations);
        System.out.println();
        
        // Benchmark 2: FastAudioPlayer (WASAPI)
        System.out.println("┌────────────────────────────────────────────────────────┐");
        System.out.println("│  Test 2: FastAudioPlayer (WASAPI Native)              │");
        System.out.println("└────────────────────────────────────────────────────────┘");
        long wasapiTime = benchmarkWasapi(wavFile, iterations);
        System.out.println();
        
        // Results
        System.out.println("╔════════════════════════════════════════════════════════════╗");
        System.out.println("║                    BENCHMARK RESULTS                      ║");
        System.out.println("╠════════════════════════════════════════════════════════════╣");
        System.out.printf ("║  Java Sound API:  %6d ms avg (Standard Java)            ║%n", javaTime);
        System.out.printf ("║  WASAPI Native:  %6d ms avg (FastAudioPlayer)        ║%n", wasapiTime);
        System.out.println("╠════════════════════════════════════════════════════════════╣");
        if (wasapiTime > 0) {
            double speedup = (double) javaTime / wasapiTime;
            System.out.printf("║  Speedup: %.1fx faster with WASAPI                      ║%n", speedup);
        }
        System.out.println("╚════════════════════════════════════════════════════════════╝");
    }
    
    static long benchmarkJavaSound(String filePath, int iterations) throws Exception {
        long totalTime = 0;
        System.out.println("  Warmup...");
        playWithJavaSound(filePath);
        
        for (int i = 1; i <= iterations; i++) {
            long start = System.nanoTime();
            playWithJavaSound(filePath);
            long end = System.nanoTime();
            long duration = (end - start) / 1_000_000;
            totalTime += duration;
            System.out.printf("  Run %d/%d: %5d ms%n", i, iterations, duration);
        }
        
        long avg = totalTime / iterations;
        System.out.println("  ────────────────────────");
        System.out.printf("  Average:  %5d ms%n", avg);
        return avg;
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
    
    static long benchmarkWasapi(String filePath, int iterations) throws Exception {
        long totalTime = 0;
        System.out.println("  Warmup...");
        playWithWasapi(filePath);
        
        for (int i = 1; i <= iterations; i++) {
            long start = System.nanoTime();
            boolean success = playWithWasapi(filePath);
            long end = System.nanoTime();
            long duration = (end - start) / 1_000_000;
            totalTime += duration;
            String status = success ? "✓" : "✗";
            System.out.printf("  Run %d/%d: %5d ms %s%n", i, iterations, duration, status);
        }
        
        long avg = totalTime / iterations;
        System.out.println("  ────────────────────────");
        System.out.printf("  Average:  %5d ms%n", avg);
        return avg;
    }
    
    static boolean playWithWasapi(String filePath) throws Exception {
        try {
            FastAudioPlayer player = new FastAudioPlayer();
            if (!player.load(filePath)) {
                player.close();
                return false;
            }
            player.setVolume(1.0f);
            if (!player.play()) {
                player.close();
                return false;
            }
            while (player.isPlaying()) {
                Thread.sleep(10);
            }
            player.close();
            return true;
        } catch (Exception e) {
            System.err.println("  Error: " + e.getMessage());
            return false;
        }
    }
    
    private static void generateBeepWav(String filename) throws Exception {
        int sampleRate = 44100;
        int duration = 1; // 1 second for benchmark
        int frequency = 440;
        int numSamples = sampleRate * duration;
        int dataSize = numSamples * 2;
        
        try (FileOutputStream out = new FileOutputStream(filename)) {
            out.write("RIFF".getBytes());
            writeInt(out, 36 + dataSize);
            out.write("WAVE".getBytes());
            out.write("fmt ".getBytes());
            writeInt(out, 16);
            writeShort(out, (short) 1);
            writeShort(out, (short) 1);
            writeInt(out, sampleRate);
            writeInt(out, sampleRate * 2);
            writeShort(out, (short) 2);
            writeShort(out, (short) 16);
            out.write("data".getBytes());
            writeInt(out, dataSize);
            
            for (int i = 0; i < numSamples; i++) {
                double time = i / (double) sampleRate;
                double sample = 0.5 * Math.sin(2 * Math.PI * frequency * time);
                short pcm = (short) (sample * 32767);
                writeShort(out, pcm);
            }
        }
    }
    
    private static void writeInt(FileOutputStream out, int value) throws Exception {
        out.write(value & 0xFF);
        out.write((value >> 8) & 0xFF);
        out.write((value >> 16) & 0xFF);
        out.write((value >> 24) & 0xFF);
    }
    
    private static void writeShort(FileOutputStream out, short value) throws Exception {
        out.write(value & 0xFF);
        out.write((value >> 8) & 0xFF);
    }
}
