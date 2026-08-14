package fastaudioplayer.demo;
import fastaudioplayer.FastAudioPlayer;

public class Demo {
    public static void main(String[] args) {
        System.out.println("--- FastAudioPlayer 0.1.2 Demo ---");
        try {
            FastAudioPlayer player = new FastAudioPlayer();
            System.out.println("Audio player initialized: " + player);
            System.out.println("✔ FastAudioPlayer demo completed.");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}