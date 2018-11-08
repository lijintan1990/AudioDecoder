//
// Created by Administrator on 2018/11/6.
//

#include "Resampler.h"
#include <chrono>
extern "C"{
#include "resample.h"
}

int Resampler::m_nOnceHandleSampleNum = 2048;
Resampler::Resampler(int srcChannels, int srcSamplerate, int dstChannels, int dstSamplerate, int64_t starttime, int type):
        m_nDstChannels(dstChannels), m_nDstSampleRate(dstSamplerate), m_nSrcChannels(srcChannels), m_nSrcSampleRete(srcSamplerate), m_nStartTime(starttime), m_nAudioType(type)
{
    // alloc 64k memory
    m_pAudioCycleBuff = make_shared<CCycleBuffer>(1<<16);
}
Resampler::~Resampler() {}

void Resampler::pushAudioData(uint8_t *data, int len, int64_t timestamp, int type = MAIN_AUDIO_STREAM) {
    shared_ptr<RawAudioData> audioData = make_shared<RawAudioData> (data, len, timestamp);
}

void Resampler::setNextHandler(shared_ptr<Mixer> next) {
    m_ptrNextHandler = next;
}

void Resampler::start(){
    m_ptrOnceResampleData = shared_ptr<uint8_t> (new uint8_t[4096], [](uint8_t* p) { delete[] p;});
    mThread = thread(this, Resampler::run);
}

void Resampler::stop() {
    m_bAbort = true;
    mThread.join();
}

int64_t Resampler::calculateAbusoluteTime(int dataLen)
{
    int64_t timestamp = dataLen * 1000 / m_nDstSampleRate + m_nStartTime;
    return timestamp;
}

void Resampler::run() {
    uint8_t* pBuf = m_ptrOnceResampleData.get();
    double factor = ((double)m_nDstSampleRate) / m_nSrcSampleRete;
    int outBufLen = factor * m_nOnceHandleSampleNum + 32;
    int64_t timestamp = 0;
    int targetTotalLen = 0;

    while(m_bAbort) {
        shared_ptr<short> ptrOutBuf(new short[outBufLen]);
        if (m_pAudioCycleBuff->read(pBuf, m_nOnceHandleSampleNum) != -1) {
            int outLen = resample_simple(factor, static_cast<short*>(pBuf), ptrOutBuf.get(), m_nOnceHandleSampleNum);
            targetTotalLen += outLen;
            timestamp = calculateAbusoluteTime(targetTotalLen);
            // deliver to Mixer module
            m_ptrNextHandler->pushAudioData(ptrOutBuf.get(), outLen, timestamp, m_nAudioType);
        } else {
            std::chrono::milliseconds t(20000);
            // gcc -D_GLIBCXX_USE_NANOSLEEP
            std::this_thread::sleep_for(t);
        }
    }
}