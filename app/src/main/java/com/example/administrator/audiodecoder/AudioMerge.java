package com.example.administrator.audiodecoder;
import android.util.Log;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

public class AudioMerge extends Thread implements AudioRawDataCallback{
    private static final String TAG = "AudioMerge";
    private ArrayList<String> mBackgroundMusicFile; //背景音mp3格式
    private ArrayList<String> mActorVoice; //配音wav格式
    private ArrayList<AudioNode> mAudioNodes;
    private Boolean mRunning = false;
    private List<AudioDecoding> mDecoderLst;
    private ReentrantLock lock = new ReentrantLock();
    private AudioDecoding.AudioFormat mFisrtAudioFormat = null;
    private AudioResample mBgmResample;
    private AudioResample mVoiceResample;
    // test
    FileOutputStream fileOutputStream = null;
    FileOutputStream voiceFileOutputStream = null;

    public AudioMerge() {
    }

    public void startMerge() {
        mRunning = true;
        this.start();
        mAudioNodes = new ArrayList<>();
        try {
            fileOutputStream = new FileOutputStream("/sdcard/bgm.pcm");
            voiceFileOutputStream = new FileOutputStream("/sdcard/voice.pcm");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
    }

    public void stopMerge() {
        try {
            fileOutputStream.flush();
            fileOutputStream.close();
            voiceFileOutputStream.flush();
            voiceFileOutputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void  putWillMergedAudio(int type, String fileName) {
        AudioNode node = new AudioNode(type, fileName);
        lock.lock();
        mAudioNodes.add(node);
        lock.unlock();
    }

    @Override
    public void run() {
        while (mRunning) {
            if (mAudioNodes.size() != 0) {
                AudioNode node = mAudioNodes.get(0);
                mAudioNodes.remove(0);
                AudioDecoding decoder = new AudioDecoding(node.mFilePath, this);

                if (!decoder.parse()) {
                    Log.e(TAG, "parse audio failed");
                    return;
                }

                // decode
                decoder.start();
            } else {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    public void DataCallBack(byte[] data, AudioDecoding.AudioFormat format) {
        if (mFisrtAudioFormat == null) {
            Log.e(TAG, "first audio format is null");
            return;
        }

        // restore pcm
        try {
            fileOutputStream.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void DataCallBack(ByteBuffer data, AudioDecoding.AudioFormat format) {

    }

    @Override
    public void FinishCallBack() {
        Log.d(TAG, "finish resample");
    }
}