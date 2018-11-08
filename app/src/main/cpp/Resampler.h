//
// Created by Administrator on 2018/11/6.
//

#ifndef AUDIODECODER_RESAMPLER_H
#define AUDIODECODER_RESAMPLER_H
#include "Mixer.h"
#include "CCycleBuffer.h"

class Resampler {
private:
    //queue<shared_ptr<RawAudioData>> m_ptrAudioQueue;
    shared_ptr<CCycleBuffer> m_pAudioCycleBuff;
    shared_ptr<Mixer> m_ptrNextHandler;
    int m_nDstChannels;
    int m_nDstSampleRate;
    int m_nSrcChannels;
    int m_nSrcSampleRete;
    int m_nAudioType;
    int64_t m_nStartTime;
    thread mThread;
    bool m_bAbort;

    shared_ptr<uint8_t> m_ptrOnceResampleData;
    static int m_nOnceHandleSampleNum;
    //RawAudioData m_audioType;
public:
    Resampler(int srcChannels, int srcSamplerate, int dstChannels, int dstSamplerate, int64_t starttime, int type);
    ~Resampler();

    void pushAudioData(uint8_t *data, int len, int64_t timestamp, int type = MAIN_AUDIO_STREAM);
    void setNextHandler(shared_ptr<Mixer> next);
    void start();
    void stop();
    void run();

private:
    int64_t calculateAbusoluteTime(int dataLen);
};

#endif //AUDIODECODER_RESAMPLER_H
