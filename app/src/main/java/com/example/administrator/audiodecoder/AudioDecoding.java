package com.example.administrator.audiodecoder;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.util.Log;

import java.io.IOException;
import java.nio.ByteBuffer;

public class AudioDecoding {
    private static final String TAG = "AudioDecording";
    private String mInFilename = null;
    private AudioFormat mAudioFormat = null;
    private MediaFormat mediaFormat = null;
    private MediaExtractor mediaExtractor = null;
    private AudioRawDataCallback mRawDataCallback;
    private boolean mRuning = false;

    class AudioFormat {
        int sampleRate;
        int channels;
    }

    /**
     * constructor
     * @param fileName
     */
    public AudioDecoding(String fileName, AudioRawDataCallback callback) {
        mInFilename = fileName;
        mRawDataCallback = callback;
    }

    public AudioFormat getMediaFormat() {
        return mAudioFormat;
    }

    public boolean parse() {
        mediaExtractor = new MediaExtractor();
        try {
            mediaExtractor.setDataSource(mInFilename);
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (int i=0; i<mediaExtractor.getTrackCount(); i++) {
            mediaFormat = mediaExtractor.getTrackFormat(i);
            String mime = mediaFormat.getString(MediaFormat.KEY_MIME);
            if (mime.startsWith("audio/")) {
                mediaExtractor.selectTrack(i);
                mAudioFormat = new AudioFormat();
                mAudioFormat.sampleRate = mediaFormat.getInteger(MediaFormat.KEY_SAMPLE_RATE);
                mAudioFormat.channels = mediaFormat.getInteger(MediaFormat.KEY_CHANNEL_COUNT);
                break;
            }
        }

        if (mAudioFormat == null) {
            Log.d(TAG, "do not found audio");
            mediaExtractor.release();
            return false;
        }

        return true;
    }

    public void start() {
        if (mRuning) {
            Log.w(TAG, "is running");
            return;
        }
        mRuning = true;
        AudioDecodeThread decodeThread = new AudioDecodeThread();
        decodeThread.start();
    }

    public void stop() {
        mRuning = false;
    }

    private void decode() throws IOException {
        String mediaMime = mediaFormat.getString(MediaFormat.KEY_MIME);
        MediaCodec codec = MediaCodec.createDecoderByType(mediaMime);
        codec.configure(mediaFormat, null, null, 0);
        codec.start();

        ByteBuffer[] codecInputBuffers = codec.getInputBuffers();
        ByteBuffer[] codecOutputBuffers = codec.getOutputBuffers();

        final double audioDurationUs = mediaFormat.getLong(MediaFormat.KEY_DURATION);
        final long kTimeOutUs = 5000;
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        boolean sawInputEOS = false;
        boolean sawOutputEOS = false;

        while (!sawOutputEOS && mRuning) {
            if (!sawInputEOS) {
                //解码器中取出一块内存索引
                int inputBufIndex = codec.dequeueInputBuffer(kTimeOutUs);
                Log.d(TAG, "inputBufIndex:" + inputBufIndex);
                if (inputBufIndex >= 0) {
                    ByteBuffer dstBuf = codecInputBuffers[inputBufIndex];
                    //extractor中读取数据到codec中去
                    int sampleSize = mediaExtractor.readSampleData(dstBuf, 0);
                    long presentationTimeUs = 0;
                    if (sampleSize < 0) {
                        Log.i(TAG, "saw input EOS");
                        sawInputEOS = true;
                        sampleSize = 0;
                    } else {
                        // 读取到数据
                        presentationTimeUs = mediaExtractor.getSampleTime();
                    }

                    codec.queueInputBuffer(inputBufIndex,
                            0,
                            sampleSize,
                            presentationTimeUs,
                            sawInputEOS ? MediaCodec.BUFFER_FLAG_END_OF_STREAM : 0);
                    mediaExtractor.advance();
                }
            }

            int res = codec.dequeueOutputBuffer(info, kTimeOutUs);
            if (res >= 0) {
                int outputBufIndex = res;
                ByteBuffer outBuf = codecOutputBuffers[outputBufIndex];
                mRawDataCallback.DataCallBack(outBuf, mAudioFormat);
                codec.releaseOutputBuffer(outputBufIndex, false);

                if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    Log.i(TAG, "saw output EOS.");
                    sawOutputEOS = true;
                }

                Log.d(TAG, "get raw sample data len:" + info.size);
            } else if (res == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                codecOutputBuffers = codec.getOutputBuffers();
                Log.i(TAG, "output buffers have changed.");
            } else if (res == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                MediaFormat oformat = codec.getOutputFormat();
                Log.i(TAG, "output format has changed to " + oformat);
            }
        }
        Log.d(TAG, "decode finish");
        mRawDataCallback.FinishCallBack();
    }


    class AudioDecodeThread extends Thread {
        @Override
        public void run() {
            super.run();
            try {
                decode();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
