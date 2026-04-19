import fastaudio.FastAudioPlayer;

public class DebugPlay {
    public static void main(String[] args) throws Exception {
        FastAudioPlayer p = new FastAudioPlayer();
        
        System.out.println("Loading: " + args[0]);
        boolean loaded = p.load(args[0]);
        System.out.println("Load result: " + loaded);
        
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
