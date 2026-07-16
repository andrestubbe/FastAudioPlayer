package fastaudio;

import fastaudio.FastAudioPlayer;
import java.io.File;
import java.io.FileOutputStream;

/**
 * FastAudioPlayer Console Demo
 * 
 * Demonstrates basic loading, volume control, device query, and real-time playback
 * with an interactive progress bar in the terminal.
 */
public class Demo {

    public static void main(String[] args) throws Exception {
        System.out.println("=================================================");
        System.out.println("⚡ FastAudioPlayer — High-Performance Console Demo ⚡");
        System.out.println("=================================================\n");

        // 1. Available devices query
        System.out.println("🎙️ Querying active Windows WASAPI devices...");
        String[] devices = FastAudioPlayer.getDevices();
        for (int i = 0; i < devices.length; i++) {
            System.out.println("  [" + i + "] " + devices[i]);
        }
        System.out.println();

        // 2. Load beep.wav (auto-generate if missing)
        String audioFile = args.length > 0 ? args[0] : "beep.wav";
        File file = new File(audioFile);
        if (!file.exists()) {
            System.out.println("🔊 '" + audioFile + "' not found. Auto-generating a clean 440Hz sine wave...");
            generateBeepWav(audioFile);
            System.out.println("  ✓ Generated successfully.\n");
        }

        // 3. Create player
        System.out.println("🛠️ Initializing native WASAPI Audio Client...");
        FastAudioPlayer player = new FastAudioPlayer();
        System.out.println("  ✓ Player created.");

        // 4. Load file
        System.out.print("📂 Loading '" + audioFile + "'... ");
        if (player.load(file)) {
            System.out.println("Success!");
        } else {
            System.out.println("Failed.");
            player.close();
            return;
        }

        long duration = player.getDuration();
        System.out.println("⏱️ Duration: " + duration + " ms");

        // 5. Volume
        player.setVolume(0.8f);
        System.out.printf("🔊 Volume set to: %.1f (80%%)%n", player.getVolume());
        System.out.println();

        // 6. Playback with animated terminal progress bar
        System.out.println("▶ Starting playback...");
        if (player.play()) {
            int width = 30; // progress bar width
            while (player.isPlaying()) {
                long pos = player.getPosition();
                double percent = (double) pos / duration;
                int filled = (int) (percent * width);
                if (filled > width) filled = width;
                
                StringBuilder sb = new StringBuilder("\r[");
                for (int i = 0; i < width; i++) {
                    if (i < filled) sb.append("=");
                    else if (i == filled) sb.append(">");
                    else sb.append(" ");
                }
                sb.append(String.format("] %d ms / %d ms", pos, duration));
                System.out.print(sb.toString());
                
                Thread.sleep(30);
            }
            
            // final 100% print
            StringBuilder sb = new StringBuilder("\r[");
            for (int i = 0; i < width; i++) sb.append("=");
            sb.append(String.format("] %d ms / %d ms", duration, duration));
            System.out.println(sb.toString());
            System.out.println("\n🎉 Playback finished successfully!");
        } else {
            System.out.println("❌ Failed to start playback.");
        }

        // 7. Cleanup
        System.out.println("🔌 Closing native resources...");
        player.close();
        System.out.println("  ✓ Done.");
    }

    private static void generateBeepWav(String filename) throws Exception {
        int sampleRate = 44100;
        int duration = 3; // 3 seconds
        int frequency = 440; // 440 Hz
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
