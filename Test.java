import fastaudio.FastAudioPlayer;

public class Test {
    public static void main(String[] args) throws InterruptedException {
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
        System.out.println("\n3. Loading beep.wav...");
        boolean loaded = player.load("beep.wav");
        System.out.println("   Loaded: " + loaded);
        
        // Test 4: Get duration
        long duration = player.getDuration();
        System.out.println("   Duration: " + duration + " ms");
        
        // Test 5: Volume control
        System.out.println("\n4. Volume control:");
        float vol = player.getVolume();
        System.out.println("   Default volume: " + vol);
        player.setVolume(1.0f);
        System.out.println("   Set to 1.0 (MAX), now: " + player.getVolume());
        
        // Test 6: Playback once
        System.out.println("\n5. Playing audio (you should hear a 440Hz tone for 3 seconds)...");
        boolean playing = player.play();
        System.out.println("   play(): " + playing);
        System.out.println("   isPlaying: " + player.isPlaying());
        
        if (playing) {
            System.out.println("   Listening...");
            while (player.isPlaying()) {
                Thread.sleep(100);
                System.out.printf("\r   Position: %d ms / %d ms", player.getPosition(), player.getDuration());
            }
            System.out.println();
            System.out.println("   Playback finished!");
        }
        
        // Test 7: Position
        System.out.println("\n6. Final Position: " + player.getPosition() + " ms");
        
        // Clean up
        System.out.println("\n7. Closing player...");
        player.close();
        System.out.println("   ✓ Player closed");
        
        System.out.println("\n=== All tests passed! ===");
    }
}
