package com.example.administrator.audiodecoder;

import java.util.List;
import java.util.Queue;

public class AudioMerge extends Thread{
    private List<String> mBackgroundMusicFile; //背景音mp3格式
    private List<String> mActorVoice; //配音wav格式
    private Queue<AudioNode> mAudioNodes;

    public AudioMerge() {
    }

    public void startMerge() {
        this.start();
    }

    public synchronized void  putWillMergeAudio(int type, String fileName) {
        AudioNode node = new AudioNode(type, fileName);

    }

    @Override
    public void run() {
        super.run();

    }
}
