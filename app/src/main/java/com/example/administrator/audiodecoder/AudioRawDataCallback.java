package com.example.administrator.audiodecoder;

public interface AudioRawDataCallback {
    void BgmDataCallBack(byte[] data);
    void VoiceDataCallBack(byte[] data);
    void DataCallbackForResample(byte[] data);
    void DataCallBackForMixer(byte[] data);
}
