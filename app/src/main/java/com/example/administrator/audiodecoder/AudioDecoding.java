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
    private MediaFormat mediaFormat = null;
    private MediaExtractor mediaExtractor = null;
    private boolean bFirstAudioFile = false;

    public AudioDecoding(String fileName, boolean firstAudioFile) {
        mInFilename = fileName;
        bFirstAudioFile = firstAudioFile;
    }

    public MediaFormat getMediaFormat() {
        return mediaFormat;
    }

    public boolean parse() {
        mediaExtractor = new MediaExtractor();
        try {
            mediaExtractor.setDataSource(mInFilename);
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (int i=0; i<mediaExtractor.getTrackCount(); i++) {
            MediaFormat format = mediaExtractor.getTrackFormat(i);
            String mime = format.getString(MediaFormat.KEY_MIME);
            if (mime.startsWith("audio/")) {
                mediaExtractor.selectTrack(i);
                mediaFormat = format;
                break;
            }
        }

        if (mediaFormat == null) {
            Log.d(TAG, "do not found audio");
            mediaExtractor.release();
            return false;
        }

        return true;
    }

    public void start() {
        AudioDecodeThread decodeThread = new AudioDecodeThread();
        decodeThread.start();
    }

    private void decode() throws IOException{
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
        int totalRawSize = 0;

        while (!sawOutputEOS) {
            if(!sawInputEOS) {
                //解码器中取出一块内存索引
                int inputBufIndex = codec.dequeueInputBuffer(kTimeOutUs);
                if (inputBufIndex >= 0) {
                    ByteBuffer dstBuf = codecInputBuffers[inputBufIndex];
                    //extractor中读取数据到codec中去
                    int sampleSize = mediaExtractor.readSampleData(dstBuf, 0);
                    if (sampleSize < 0) {
                        Log.i(TAG, "saw input EOS");
                        sawInputEOS = true;
                        codec.queueInputBuffer(inputBufIndex,
                                0,
                                0,
                                0,
                                MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                    } else {
                        // 读取到数据
                        long presentationTimeUs = mediaExtractor.getSampleTime();
                        codec.queueInputBuffer(inputBufIndex,
                                0,
                                sampleSize,
                                presentationTimeUs,
                                0);
                        mediaExtractor.advance();
                    }

                    int res = codec.dequeueOutputBuffer(info, kTimeOutUs);
                    if (res >= 0) {
                        int outputBufIndex = res;
                        ByteBuffer outBuf = codecOutputBuffers[outputBufIndex];
                        outBuf.position(info.offset);
                        outBuf.limit(info.offset + info.size);
                        byte[] data = new byte[info.size];
                        outBuf.get(data);
                        Log.d(TAG, "get raw sample data len:" + info.size);
                    }else if (res == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                        codecOutputBuffers = codec.getOutputBuffers();
                        Log.i(TAG, "output buffers have changed.");
                    } else if (res == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                        MediaFormat oformat = codec.getOutputFormat();
                        Log.i(TAG, "output format has changed to " + oformat);
                    }
                }
            }
        }
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
