//
// Created by Administrator on 2018/11/5.
//

#include "Mixer.h"
#include "AILog.h"
#include <chrono>
#include <stdlib.h>

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

void Mixer::start() {
    if (!m_musicLst.empty() || !m_voiceLst.empty()) {
        mThread = thread(this, Mixer::mixFileRun);
    }
//    else {
//        mThread = thread(this, Mixer::run);
//    }
}

void Mixer::stop() {
    m_bAbort = true;
}

void Mixer::mixData(short *data1, short *data2, int rawDataCnt)
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

int Mixer::rescaleAudioLenToMillsec(int audioLen)
{
    return 1000 * audioLen / 44100;
}

int Mixer::rescaleAudioMillsecToLen(int timestamp)
{
    return timestamp * 44100 / 1000;
}

void Mixer::preProcessAddMuteData(list<shared_ptr<AudioFileMsg>>::iterator &iter, int &mixedLen)
{
    uint8_t data[2048] = {0};
    // 检查是否需要加静音包
    int beginLen = rescaleAudioMillsecToLen((*iter)->startTime);
    if (beginLen > mixedLen) {
        // 计算需要加入多长的静音包
        int l = beginLen - mixedLen;
        while(l > sizeof(data)) {
            deliverOutRawData(data, sizeof(data));
            l -= sizeof(data);
            mixedLen += sizeof(data);
        }
        if (l > 0) {
            deliverOutRawData(data, l);
            mixedLen += l;
        }
    }
}
void Mixer::proccessSingleFile(list<shared_ptr<AudioFileMsg>> &lst, int& mixedLen)
{
    uint8_t data[2048] = {0};
    // 只有背景音
    list<shared_ptr<AudioFileMsg>>::iterator iter = lst.begin();
    preProcessAddMuteData(iter, mixedLen);

    FILE* pfile = fopen((*iter)->filepath.c_str(), "rb+");
    if (pfile == NULL) {
        LOGD("open %s failed", (*iter)->filepath.c_str());
        //需要处理错误，是直接忽略还是提示上层合成失败
        return;
    }

    int ret = 0;
    do {
        ret = fread(data, 1, sizeof(data), pfile);
        if (ret <= 0) {
            LOGW("fread return %d", ret);
            break;
        }
        mixedLen += ret;
        deliverOutRawData(data, rescaleAudioLenToMillsec(mixedLen));
    } while (!feof(pfile));
    lst.erase(iter);
}

static int getFileLength(FILE* pfile)
{
    fseek(pfile, 0, SEEK_END);
    return ftell(pfile);
}

void Mixer::mixFileRun()
{
    LOGD("mixFileRun");
    //已经混音了的音频长度
    int mixedLen = 0;
    while (!m_bAbort) {
        if (m_musicLst.empty() && m_voiceLst.empty()) {
            m_bAbort = true;
            break;
        }

        if (m_voiceLst.empty()) {
            //只有背景音
            proccessSingleFile(m_musicLst, mixedLen);
        } else if (m_musicLst.empty()) {
            //只有配音
            proccessSingleFile(m_voiceLst, mixedLen);
        } else {
            //有配音有背景音
            list<shared_ptr<AudioFileMsg>>::iterator voiceIter = m_voiceLst.begin();
            list<shared_ptr<AudioFileMsg>>::iterator musicIter = m_musicLst.begin();

            if ((*voiceIter)->startTime > (*musicIter)->startTime) {
                //加静音
                preProcessAddMuteData(musicIter, mixedLen);

                if ((*musicIter)->startTime + (*musicIter)->duration > (*voiceIter)->startTime) {
                    // 获取不需要混音的数据长度
                    int firstlen = rescaleAudioMillsecToLen((*voiceIter)->startTime) - mixedLen;

                    FILE* pfile = fopen((*voiceIter)->filepath.c_str(), "rb+");
                    if (pfile == NULL) {
                        LOGD("open %s failed", (*voiceIter)->filepath.c_str());
                        //需要处理错误，是直接忽略还是提示上层合成失败
                        return;
                    }

                    uint8_t data[2048] = {0};
                    int readLen = sizeof(data);
                    int ret = 0;
                    while(firstlen > readLen) {
                        ret = fread(data, 1, readLen, pfile);
                        if (ret <= 0) {
                            LOGW("fread return %d", ret);
                            break;
                        }
                        mixedLen += ret;
                        firstlen -= ret;
                        deliverOutRawData(data, rescaleAudioLenToMillsec(mixedLen));
                    }
                    // 检查是否可以继续读，便于处理提前一点进行混音
                    if () {

                    }
                }
            } else {

            }
        }
    }
}

void deliverOutRawData(uint8_t *data, int timestamp)
{

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