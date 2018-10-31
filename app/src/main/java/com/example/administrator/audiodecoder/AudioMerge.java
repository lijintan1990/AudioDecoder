package com.example.administrator.audiodecoder;

import android.media.MediaFormat;
import android.util.Log;
import android.view.autofill.AutofillId;

import java.util.List;
import java.util.Queue;
import java.util.concurrent.locks.ReentrantLock;

public class AudioMerge extends Thread implements AudioRawDataCallback{
    private static final String TAG = "AudioMerge";
    private List<String> mBackgroundMusicFile; //背景音mp3格式
    private List<String> mActorVoice; //配音wav格式
    private Queue<AudioNode> mAudioNodes;
    private Boolean mRunning = false;
    private List<AudioDecoding> mDecoderLst;
    private ReentrantLock lock = new ReentrantLock();
    private AudioDecoding.AudioFormat mFisrtAudioFormat = null;

    public AudioMerge() {
    }

    public void startMerge() {
        mRunning = true;
        this.start();
    }

    public void stopMerge() {

    }

    public void  putWillMergedAudio(int type, String fileName) {
        AudioNode node = new AudioNode(type, fileName);
        lock.lock();
        mAudioNodes.add(node);
        lock.unlock();
    }

    @Override
    public void run() {
        Boolean bFirstAudio = true;
        while (mRunning) {
            if (mAudioNodes.size() != 0) {
                AudioNode node = mAudioNodes.peek();
                if (node.mFileType == AudioNode.BGM_TYPE) {
                    AudioDecoding decoder = new AudioDecoding(node.mFilePath, bFirstAudio, AudioNode.BGM_TYPE);

                    if(!decoder.parse()) {
                        Log.e(TAG, "parse audio failed");
                        return;
                    }
                    if (bFirstAudio) {
                        mFisrtAudioFormat = decoder.getMediaFormat();
                    }

                    // decode
                    decoder.start();

                    if (!mFisrtAudioFormat.equals(decoder.getMediaFormat())) {
                        // resample
                        AudioResample audioResample = new AudioResample(mFisrtAudioFormat);

                    }

                    bFirstAudio = false;
                }
            } else {
                try {
                    Thread.sleep(20000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    @Override
    public void BgmDataCallBack(byte[] data) {

    }

    @Override
    public void VoiceDataCallBack(byte[] data) {

    }
}
