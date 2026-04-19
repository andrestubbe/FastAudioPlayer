import fastaudio.FastAudioPlayer;

public class DebugPlay {
    public static void main(String[] args) throws Exception {
        System.out.println("[Java] Creating player...");
        FastAudioPlayer p = new FastAudioPlayer();
        System.out.println("[Java] Player created, loading: " + args[0]);
        
        boolean loaded = p.load(args[0]);
        System.out.println("[Java] Load result: " + loaded);
        
        if (loaded) {
            System.out.println("Duration: " + p.getDuration() + " ms");
            System.out.println("Playing...");
            p.play();
            while (p.isPlaying()) {
                System.out.print(".");
                Thread.sleep(200);
            }
            System.out.println("\nDone!");
            p.close();
        }
    }
}
