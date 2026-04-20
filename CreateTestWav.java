import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Creates a simple test WAV file with a 440Hz sine wave (A4 note)
 */
public class CreateTestWav {
    
    public static void main(String[] args) throws IOException {
        String filename = "test.wav";
        int sampleRate = 44100;
        int durationSeconds = 3;
        int frequency = 440; // A4 note
        
        int numSamples = sampleRate * durationSeconds;
        int byteRate = sampleRate * 2; // 16-bit mono = 2 bytes per sample
        int dataSize = numSamples * 2;
        
        try (FileOutputStream out = new FileOutputStream(filename)) {
            // RIFF header
            out.write("RIFF".getBytes());
            writeInt(out, 36 + dataSize); // file size - 8
            out.write("WAVE".getBytes());
            
            // fmt chunk
            out.write("fmt ".getBytes());
            writeInt(out, 16); // chunk size
            writeShort(out, (short) 1); // audio format (PCM)
            writeShort(out, (short) 1); // channels (mono)
            writeInt(out, sampleRate);
            writeInt(out, byteRate);
            writeShort(out, (short) 2); // block align
            writeShort(out, (short) 16); // bits per sample
            
            // data chunk
            out.write("data".getBytes());
            writeInt(out, dataSize);
            
            // Generate sine wave
            for (int i = 0; i < numSamples; i++) {
                double time = i / (double) sampleRate;
                // 440Hz sine wave with 0.5 amplitude (not too loud)
                double sample = 0.5 * Math.sin(2 * Math.PI * frequency * time);
                // Convert to 16-bit PCM
                short pcm = (short) (sample * 32767);
                writeShort(out, pcm);
            }
        }
        
        System.out.println("Created: " + filename);
        System.out.println("Duration: " + durationSeconds + " seconds");
        System.out.println("Frequency: " + frequency + " Hz (A4 note)");
    }
    
    private static void writeInt(FileOutputStream out, int value) throws IOException {
        out.write(value & 0xFF);
        out.write((value >> 8) & 0xFF);
        out.write((value >> 16) & 0xFF);
        out.write((value >> 24) & 0xFF);
    }
    
    private static void writeShort(FileOutputStream out, short value) throws IOException {
        out.write(value & 0xFF);
        out.write((value >> 8) & 0xFF);
    }
}
