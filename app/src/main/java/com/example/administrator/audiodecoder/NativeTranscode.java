package com.example.administrator.audiodecoder;

import java.nio.ByteBuffer;

public class NativeTranscode {
    private long nativeResampler = 0;

    public void initResampler(String outFilepath, int samples, int inChannels,
                                     int inSampleRate, int sampleFmt, int outChannnels,
                                     int outSampleRate, int outSampleFmt)
    {
        nativeResampler =  nativeInitResampler(outFilepath, samples, inChannels,
                    inSampleRate, sampleFmt, outChannnels,
                    outSampleRate, outSampleFmt);
    }

    public void endResample() {
        if (nativeResampler == 0)
            return;
        endResample(nativeResampler);
    }

    public  void pushAudioData(ByteBuffer buffer, int bytelen)
    {
        if (nativeResampler == 0)
            return;
        nativePushAudioData(nativeResampler, buffer, bytelen);
    }

    private native long nativeInitResampler(String outFilepath, int samples, int inChannels,
                                      int inSampleRate, int sampleFmt, int outChannnels,
                                      int outSampleRate, int outSampleFmt);

    private native void endResample(long obj);
    private native void nativePushAudioData(long obj, ByteBuffer buffer, int bytelen);
}
