//
// Created by ljt on 2018/11/12.
//

#ifndef AUDIODECODER_AUDIORESAMPLE_H
#define AUDIODECODER_AUDIORESAMPLE_H

#include <string>
#include <memory>
#include <thread>
#include "CCycleBuffer.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavutil/opt.h"
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

class AudioResample {
public:
    AudioResample() {
        m_pCycleBuffer = nullptr;
        m_pFile = nullptr;
        swr_ctx = nullptr;
        m_bRunning = false;
        m_pSrcData = nullptr;
        m_pDstData = nullptr;
    }
    virtual ~AudioResample() {

    }
    int initResample(const char* outFilepath, int samples, int inChannels, int inSampleRate,
                      int sampleFmt, int outChannnels, int outSampleRate, int outSampleFmt);
    int resample();
    void pushAudioData(uint8_t* data, int len);
    void end();
    void run();

private:
    void release();
private:
    bool m_bRunning;
    int m_nSrcChannels;
    int m_nSrcSampleRate;
    int m_nSrcSampleFmt;

    int m_nDstChannels;
    int m_nDstSampleRate;
    int m_nDstSampleFmt;

    int m_nMaxDstNumSamples;
    int m_nbSamples;
    uint8_t **m_pSrcData;
    uint8_t **m_pDstData;
    std::string m_strOutFilepath;
    FILE* m_pFile;
    struct SwrContext *swr_ctx;
    std::thread m_thread;
    shared_ptr<CCycleBuffer> m_pCycleBuffer;
};


#endif //AUDIODECODER_AUDIORESAMPLE_H
