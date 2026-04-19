import fastaudio.FastAudioPlayer;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import java.awt.*;
import java.awt.event.*;
import java.io.File;

/**
 * Audio Player GUI Example
 * 
 * Demonstrates FastAudioPlayer API with a simple Swing interface.
 * Plays WAV and MP3 files.
 * 
 * Note: v1.0 uses stub implementation. Real WASAPI playback in v1.1.
 */
public class AudioPlayerGUI extends JFrame {
    
    private FastAudioPlayer player;
    private JLabel statusLabel;
    private JLabel fileLabel;
    private JLabel timeLabel;
    private JSlider volumeSlider;
    private JButton playButton;
    private JButton pauseButton;
    private JButton stopButton;
    private JButton loadButton;
    private JProgressBar progressBar;
    private Timer progressTimer;
    private String currentFile = null;
    
    public AudioPlayerGUI() {
        setTitle("FastAudioPlayer - Player Demo");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(500, 350);
        setLocationRelativeTo(null);
        
        initComponents();
        initAudio();
    }
    
    private void initComponents() {
        setLayout(new BorderLayout(10, 10));
        
        // Header
        JLabel header = new JLabel("🎵 Audio Player", JLabel.CENTER);
        header.setFont(new Font("Segoe UI", Font.BOLD, 24));
        header.setBorder(BorderFactory.createEmptyBorder(20, 0, 10, 0));
        add(header, BorderLayout.NORTH);
        
        // Center panel
        JPanel centerPanel = new JPanel(new GridLayout(5, 1, 5, 10));
        centerPanel.setBorder(BorderFactory.createEmptyBorder(10, 30, 10, 30));
        
        // File info
        fileLabel = new JLabel("No file loaded", JLabel.CENTER);
        fileLabel.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        centerPanel.add(fileLabel);
        
        // Time display
        timeLabel = new JLabel("00:00 / 00:00", JLabel.CENTER);
        timeLabel.setFont(new Font("Segoe UI", Font.MONOSPACED, 16));
        timeLabel.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
        centerPanel.add(timeLabel);
        
        // Progress bar
        progressBar = new JProgressBar(0, 100);
        progressBar.setStringPainted(false);
        progressBar.setValue(0);
        centerPanel.add(progressBar);
        
        // Volume
        JPanel volumePanel = new JPanel(new BorderLayout(5, 0));
        volumePanel.add(new JLabel("Vol:"), BorderLayout.WEST);
        volumeSlider = new JSlider(0, 100, 80);
        volumeSlider.addChangeListener(e -> updateVolume());
        volumePanel.add(volumeSlider, BorderLayout.CENTER);
        centerPanel.add(volumePanel);
        
        // Control buttons
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 10, 0));
        
        loadButton = new JButton("📂 Load");
        loadButton.setFont(new Font("Segoe UI", Font.BOLD, 14));
        loadButton.addActionListener(e -> loadFile());
        buttonPanel.add(loadButton);
        
        playButton = new JButton("▶ Play");
        playButton.setFont(new Font("Segoe UI", Font.BOLD, 14));
        playButton.setBackground(new Color(40, 167, 69));
        playButton.setForeground(Color.WHITE);
        playButton.setFocusPainted(false);
        playButton.setEnabled(false);
        playButton.addActionListener(e -> play());
        buttonPanel.add(playButton);
        
        pauseButton = new JButton("⏸ Pause");
        pauseButton.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        pauseButton.setEnabled(false);
        pauseButton.addActionListener(e -> pause());
        buttonPanel.add(pauseButton);
        
        stopButton = new JButton("⏹ Stop");
        stopButton.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        stopButton.setEnabled(false);
        stopButton.addActionListener(e -> stop());
        buttonPanel.add(stopButton);
        
        centerPanel.add(buttonPanel);
        add(centerPanel, BorderLayout.CENTER);
        
        // Status
        statusLabel = new JLabel("Ready - Load an audio file", JLabel.CENTER);
        statusLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        statusLabel.setBorder(BorderFactory.createEmptyBorder(5, 0, 5, 0));
        add(statusLabel, BorderLayout.SOUTH);
        
        // Footer
        JLabel footer = new JLabel("FastAudioPlayer v1.0.0 - Stub Demo (Real audio in v1.1)", JLabel.CENTER);
        footer.setFont(new Font("Segoe UI", Font.ITALIC, 11));
        footer.setForeground(Color.GRAY);
        footer.setBorder(BorderFactory.createEmptyBorder(0, 0, 10, 0));
        add(footer, BorderLayout.PAGE_END);
        
        // Progress update timer
        progressTimer = new Timer(100, e -> updateProgress());
    }
    
    private void initAudio() {
        try {
            player = new FastAudioPlayer();
            
            // Show available devices
            String[] devices = FastAudioPlayer.getDevices();
            System.out.println("Available devices:");
            for (String device : devices) {
                System.out.println("  - " + device);
            }
            
        } catch (Exception e) {
            statusLabel.setText("Error initializing: " + e.getMessage());
            loadButton.setEnabled(false);
        }
    }
    
    private void loadFile() {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileFilter(new FileNameExtensionFilter("Audio files", "wav", "mp3"));
        
        if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            File file = chooser.getSelectedFile();
            currentFile = file.getAbsolutePath();
            
            try {
                if (player.load(currentFile)) {
                    fileLabel.setText(file.getName());
                    statusLabel.setText("Loaded: " + file.getName());
                    playButton.setEnabled(true);
                    pauseButton.setEnabled(false);
                    stopButton.setEnabled(false);
                    
                    long duration = player.getDuration();
                    timeLabel.setText("00:00 / " + formatTime(duration));
                    progressBar.setValue(0);
                } else {
                    statusLabel.setText("Failed to load file");
                }
            } catch (Exception e) {
                statusLabel.setText("Error: " + e.getMessage());
            }
        }
    }
    
    private void play() {
        try {
            if (player.play()) {
                statusLabel.setText("Playing...");
                playButton.setEnabled(false);
                pauseButton.setEnabled(true);
                stopButton.setEnabled(true);
                progressTimer.start();
            } else {
                statusLabel.setText("Failed to play");
            }
        } catch (Exception e) {
            statusLabel.setText("Error: " + e.getMessage());
        }
    }
    
    private void pause() {
        try {
            if (player.pause()) {
                statusLabel.setText("Paused");
                playButton.setEnabled(true);
                pauseButton.setEnabled(false);
                progressTimer.stop();
            }
        } catch (Exception e) {
            statusLabel.setText("Error: " + e.getMessage());
        }
    }
    
    private void stop() {
        try {
            if (player.stop()) {
                statusLabel.setText("Stopped");
                playButton.setEnabled(true);
                pauseButton.setEnabled(false);
                stopButton.setEnabled(false);
                progressTimer.stop();
                progressBar.setValue(0);
                timeLabel.setText("00:00 / " + formatTime(player.getDuration()));
            }
        } catch (Exception e) {
            statusLabel.setText("Error: " + e.getMessage());
        }
    }
    
    private void updateVolume() {
        if (player != null) {
            float volume = volumeSlider.getValue() / 100.0f;
            player.setVolume(volume);
        }
    }
    
    private void updateProgress() {
        long position = player.getPosition();
        long duration = player.getDuration();
        
        if (duration > 0) {
            int percent = (int) ((position * 100) / duration);
            progressBar.setValue(percent);
        }
        
        timeLabel.setText(formatTime(position) + " / " + formatTime(duration));
        
        if (!player.isPlaying()) {
            progressTimer.stop();
            playButton.setEnabled(true);
            pauseButton.setEnabled(false);
            stopButton.setEnabled(false);
            statusLabel.setText("Finished");
        }
    }
    
    private String formatTime(long ms) {
        long seconds = ms / 1000;
        long minutes = seconds / 60;
        seconds = seconds % 60;
        return String.format("%02d:%02d", minutes, seconds);
    }
    
    @Override
    public void dispose() {
        progressTimer.stop();
        if (player != null) {
            player.close();
        }
        super.dispose();
    }
    
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            try {
                UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            } catch (Exception e) {
                e.printStackTrace();
            }
            new AudioPlayerGUI().setVisible(true);
        });
    }
}
