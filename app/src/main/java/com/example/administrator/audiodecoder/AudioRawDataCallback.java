package com.example.administrator.audiodecoder;

import java.nio.ByteBuffer;

public interface AudioRawDataCallback {
    void DataCallBack(byte[] data, AudioDecoding.AudioFormat format);
    void DataCallBack(ByteBuffer data, AudioDecoding.AudioFormat format);
    void FinishCallBack();
}
