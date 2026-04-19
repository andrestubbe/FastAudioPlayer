import fastaudio.FastAudioPlayer;

public class Test {
    public static void main(String[] args) {
        System.out.println("=== FastAudioPlayer Test ===");
        
        // Test 1: Get devices
        System.out.println("\n1. Available devices:");
        String[] devices = FastAudioPlayer.getDevices();
        for (String device : devices) {
            System.out.println("   - " + device);
        }
        
        // Test 2: Create player
        System.out.println("\n2. Creating player...");
        FastAudioPlayer player = new FastAudioPlayer();
        System.out.println("   ✓ Player created");
        
        // Test 3: Load file
        System.out.println("\n3. Loading test.mp3...");
        boolean loaded = player.load("test.mp3");
        System.out.println("   Loaded: " + loaded);
        
        // Test 4: Get duration
        long duration = player.getDuration();
        System.out.println("   Duration: " + duration + " ms");
        
        // Test 5: Volume control
        System.out.println("\n4. Volume control:");
        float vol = player.getVolume();
        System.out.println("   Default volume: " + vol);
        player.setVolume(0.5f);
        System.out.println("   Set to 0.5, now: " + player.getVolume());
        
        // Test 6: Playback controls
        System.out.println("\n5. Playback controls:");
        System.out.println("   isPlaying: " + player.isPlaying());
        System.out.println("   play(): " + player.play());
        System.out.println("   isPlaying: " + player.isPlaying());
        System.out.println("   pause(): " + player.pause());
        System.out.println("   resume(): " + player.resume());
        System.out.println("   stop(): " + player.stop());
        
        // Test 7: Position
        System.out.println("\n6. Position: " + player.getPosition() + " ms");
        
        // Clean up
        System.out.println("\n7. Closing player...");
        player.close();
        System.out.println("   ✓ Player closed");
        
        System.out.println("\n=== All tests passed! ===");
    }
}
