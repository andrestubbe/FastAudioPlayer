import javax.sound.sampled.*;
import java.io.File;

public class CheckWav {
    public static void main(String[] args) throws Exception {
        File file = new File("beep.wav");
        AudioInputStream ais = AudioSystem.getAudioInputStream(file);
        AudioFormat format = ais.getFormat();
        
        System.out.println("Format: " + format);
        System.out.println("Sample Rate: " + format.getSampleRate());
        System.out.println("Channels: " + format.getChannels());
        System.out.println("Sample Size: " + format.getSampleSizeInBits());
        System.out.println("Frame Length: " + ais.getFrameLength());
        
        byte[] buffer = new byte[1024];
        int read;
        int totalSamples = 0;
        int maxAmplitude = 0;
        
        while ((read = ais.read(buffer)) > 0) {
            for (int i = 0; i < read - 1; i += 2) {
                short sample = (short) ((buffer[i+1] << 8) | (buffer[i] & 0xFF));
                maxAmplitude = Math.max(maxAmplitude, Math.abs(sample));
                totalSamples++;
            }
        }
        
        System.out.println("Total samples: " + totalSamples);
        System.out.println("Max amplitude: " + maxAmplitude);
        System.out.println(maxAmplitude > 1000 ? "WAV HAS AUDIO" : "WAV IS SILENT!");
        
        ais.close();
    }
}
