//
// Created by Administrator on 2018/11/5.
//

#include "Mixer.h"
#include <chrono>
#include <stdlib.h>
#include "resample.h"

void Mixer::pushAudioData(short *data, int len, int timestamp, int type) {
    shared_ptr<RawAudioData> audioData = make_shared<RawAudioData> (data, len, timestamp);

    if (type == MAIN_AUDIO_STREAM) {
        m_ptrMainQueue.push(audioData);
    } else {
        m_ptrSubQueue.push(audioData);
    }
}

void Mixer::getMixedData(short *data, int &len) {

}

void Mixer::start(){
    mThread = thread(this, Mixer::run);
}

void Mixer::stop() {
    m_bAbort = true;
}

void Mixer::mix(short *data1, short *data2, int rawDataCnt)
{
    int sum = 0;
    double decayFactor = 1;

    for (int i=0; i< rawDataCnt; i++) {
        sum = (int)data1[i] + data2[i];
        sum *= decayFactor;

        // 计算衰减因子
        // 1. 叠加之后，会溢出，计算溢出的倍数（即衰减因子）
        if (sum > AUDIO_DATA_TYPE_MAX)
        {
            decayFactor = static_cast<double>(AUDIO_DATA_TYPE_MAX) / static_cast<double>(sum);  // 算大了，就用小数0.8衰减
            sum = AUDIO_DATA_TYPE_MAX;
        }
        else if (sum < AUDIO_DATA_TYPE_MIN)
        {
            decayFactor = static_cast<double>(AUDIO_DATA_TYPE_MIN) / static_cast<double>(sum);  // 算小了，就用大数1.2增加
            sum = AUDIO_DATA_TYPE_MIN;
        }

        // 2. 衰减因子的平滑（为了防止个别点偶然的溢出）
        if (decayFactor < 1)
        {
            decayFactor += static_cast<double>(1 - decayFactor) / static_cast<double>(32);
        }

        data1[i] = (short)sum;
    }
}

void Mixer::AddAndNormalization(vector<vector<short>> allMixingSounds, int   RawDataCnt, vector<short>* __pRawDataBuffer)
{
    int Sum = 0;                                    // 用更大的范围来表示（用有符号的int，而不要用无符号的DWORD）
    double decayFactor = 1;                                     // 衰减因子（防止溢出）

    for (int i = 0; i < RawDataCnt; ++i)
    {
        Sum = 0;                                                // 复位叠加的值
        for (int wavNum = 0; wavNum < allMixingSounds.size(); ++wavNum)
        {
            Sum += allMixingSounds[wavNum][i];
        }
        Sum *= decayFactor;                                     // 将衰减因子作用在叠加的音频上

        // 计算衰减因子
        // 1. 叠加之后，会溢出，计算溢出的倍数（即衰减因子）
        if (Sum > AUDIO_DATA_TYPE_MAX)
        {
            decayFactor = static_cast<double>(AUDIO_DATA_TYPE_MAX) / static_cast<double>(Sum);  // 算大了，就用小数0.8衰减
            Sum = AUDIO_DATA_TYPE_MAX;
        }
        else if (Sum < AUDIO_DATA_TYPE_MIN)
        {
            decayFactor = static_cast<double>(AUDIO_DATA_TYPE_MIN) / static_cast<double>(Sum);  // 算小了，就用大数1.2增加
            Sum = AUDIO_DATA_TYPE_MIN;
        }

        // 2. 衰减因子的平滑（为了防止个别点偶然的溢出）
        if (decayFactor < 1)
        {
            decayFactor += static_cast<double>(1 - decayFactor) / static_cast<double>(32);
        }

        __pRawDataBuffer->push_back(short(Sum));      // 把int再强制转换回为short
    }
}

void Mixer::run() {
    shared_ptr<RawAudioData> mainAudio = nullptr;
    shared_ptr<RawAudioData> subAudio = nullptr;
    shared_ptr<RawAudioData> mixedAudio = nullptr;

    short *mainMixBuff = nullptr;
    short *subMixBuff = nullptr;

    while(m_bAbort) {
        if (!m_ptrMainQueue.empty()) {
            mainAudio = m_ptrMainQueue.front();
            if (!m_ptrSubQueue.empty()) {
                subAudio = m_ptrSubQueue.front();

                //TODO:s 小于100 ms, 则开始合并, Android层线程一定要控制好，这里会有bug
                if (abs(subAudio->timestamp - mainAudio->timestamp) < 100) {

                }
            } else {
                m_ptrMixQueue.push(mainAudio);
                chrono::milliseconds t(20000);
                this_thread::sleep_for(t);
                continue;
            }
        } else {
            chrono::milliseconds t(20000);
            this_thread::sleep_for(t);
        }
    }
}