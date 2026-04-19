import fastaudio.FastAudioPlayer;

public class PlayTest {
    public static void main(String[] args) throws Exception {
        FastAudioPlayer p = new FastAudioPlayer();
        if (p.load(args[0])) {
            System.out.println("Playing...");
            p.play();
            while (p.isPlaying()) {
                System.out.printf("\r%s / %s", format(p.getPosition()), format(p.getDuration()));
                Thread.sleep(100);
            }
            System.out.println("\nDone!");
            p.close();
        } else {
            System.out.println("Failed to load");
        }
    }
    
    static String format(long ms) {
        long s = ms/1000, m = s/60;
        return String.format("%02d:%02d", m, s%60);
    }
}
