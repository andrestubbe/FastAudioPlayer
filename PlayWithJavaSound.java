import javax.sound.sampled.*;
import java.io.File;

public class PlayWithJavaSound {
    public static void main(String[] args) throws Exception {
        System.out.println("Playing beep.wav with standard Java Sound API...");
        
        File file = new File("beep.wav");
        AudioInputStream ais = AudioSystem.getAudioInputStream(file);
        AudioFormat format = ais.getFormat();
        
        System.out.println("Format: " + format);
        
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
        
        System.out.println("Finished playing!");
    }
}
