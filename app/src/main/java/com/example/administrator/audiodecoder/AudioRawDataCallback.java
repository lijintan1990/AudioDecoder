package com.example.administrator.audiodecoder;

public interface AudioRawDataCallback {
    void BgmDataCallBack(byte[] data, boolean needResample, AudioDecoding.AudioFormat format);
    void VoiceDataCallBack(byte[] data, boolean needResample, AudioDecoding.AudioFormat format);
    void DataCallbackForResample(byte[] data);
    void DataCallBackForMixer(byte[] data);
}
