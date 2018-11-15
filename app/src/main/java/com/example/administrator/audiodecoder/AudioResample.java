package com.example.administrator.audiodecoder;

import android.util.Log;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

/**
 * 重采样音频文件
 */
public class AudioResample implements AudioRawDataCallback{
    private static final String TAG = "AudioResample";
    private String mInfileName = null;
    private String mOutfileName = null;
    private final int targetSampleRate = 44100;
    private final int targetChannels = 2;
    private FileChannel fout = null;
    private AudioDecoding mDecoder;
    NativeTranscode nativeTranscode = null;
    public AudioResample(String inFile, String outFile) {
        mInfileName = inFile;
        mOutfileName = outFile;
    }

    void start() {
        mDecoder = new AudioDecoding(mInfileName, this);
        if (!mDecoder.parse()) {
            Log.e(TAG, "parse audio failed");
            return;
        }
        mDecoder.start();
    }

    void stop() {
        if (mDecoder != null)
            mDecoder.stop();
        if(nativeTranscode != null) {
            nativeTranscode.endResample();
        }

        if (fout != null) {
            try {
                fout.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void DataCallBack(byte[] data, AudioDecoding.AudioFormat format) {

    }

    @Override
    public void DataCallBack(ByteBuffer data, AudioDecoding.AudioFormat format) {
        if (format.sampleRate != targetSampleRate || format.channels != targetChannels) {
            // need resample
            if (nativeTranscode == null) {
                nativeTranscode = new NativeTranscode();
                nativeTranscode.initResampler(mOutfileName, 1024, format.channels,
                        format.sampleRate, 1, targetChannels, targetSampleRate, 1);
            }

            nativeTranscode.pushAudioData(data, data.remaining());
        } else {
            // no need to resample, write data to file
            if (fout == null) {
                try {
                    fout = new FileOutputStream(mOutfileName).getChannel();
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                    Log.e(TAG, "FileOutputStream failed");
                    return;
                }
            }

            try {
                fout.write(data);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void FinishCallBack() {
        Log.d(TAG, "decode finish");
        stop();
    }
}
