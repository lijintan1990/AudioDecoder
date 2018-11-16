//
// Created by Administrator on 2018/11/5.
//

#ifndef MIXER_H
#define MIXER_H


#include <memory>
#include <queue>
#include <thread>
#include <list>
using namespace std;

/**
 * 上层传递pcm数据，进行重采样，
 * 重采样之后把数据传递到队列，底层进行混音，
 * 混音之后需要传回到Android层的AudioEncoder，
 * 需要处理时间问题，比如2-5秒需要混音，那么2-5秒需要确保两路流都是有数据的，
 * 合理的处理方式是上层的数据通过mixer里面的线程进行合并，保存合并后的数据到队列，
 * Android层取队列中的数据
 * 这里需要存储每一段音频的开始时间和结束时间，然后需要在没有音频的时间段插入静音包
 * 静音包应该在Android的mergeAudio进行插入控制，这里只负责合成，无需关心业务逻辑
 */
#define AUDIO_DATA_TYPE_MAX 32767
#define AUDIO_DATA_TYPE_MIN -32768

enum AUDIO_TYPE {
    MAIN_AUDIO_STREAM = 0,
    SUB_AUDIO_STREAM
};

struct RawAudioData {
    short *data;
    int len;
    int type;
    int64_t timestamp;
    RawAudioData(short* data, int len, int64_t timestamp, int type) {
        this->data = data;
        this->len = len;
        this->timestamp = timestamp;
        this->type = type;
    }
};

struct AudioFileMsg {
    string filepath;
    int startTime;
    int duration;
    AUDIO_TYPE type;
    AudioFileMsg(string &filepath, int startTime, int duration, AUDIO_TYPE type)
    {
        this->filepath = filepath;
        this->startTime = startTime;
        this->duration = duration;
        this->type = type;
    }
};

class Mixer {
private:
    thread mThread;
    bool m_bAbort;
    int m_nTotalDuration; //总时长
    queue<shared_ptr<RawAudioData>> m_ptrMainQueue;
    queue<shared_ptr<RawAudioData>> m_ptrSubQueue;
    queue<shared_ptr<RawAudioData>> m_ptrMixQueue;

    list<shared_ptr<AudioFileMsg>> m_musicLst;
    list<shared_ptr<AudioFileMsg>> m_voiceLst;
public:
    Mixer(list<shared_ptr<AudioFileMsg>> &musiceLst, list<shared_ptr<AudioFileMsg>> &voiceLst, int totalDuration) {
        m_musicLst = musiceLst;
        m_voiceLst = voiceLst;
        m_nTotalDuration = totalDuration;
    }
    virtual ~Mixer() {}

    void pushAudioData(short *data, int len, int timestamp, int type);
    void getMixedData(short *data, int &len);
    void start();
    void stop();
private:
    void run();
    void mixFileRun();
    /**
     * 把数据回调给Android层
     * @param data
     * @param timestamp
     */
    void deliverOutRawData(uint8_t *data, int timestamp);
    int rescaleAudioLenToMillsec(int audioLen);
    int rescaleAudioMillsecToLen(int timestamp)
    void AddAndNormalization(vector<vector<short>> allMixingSounds, int   RawDataCnt, vector<short>* __pRawDataBuffer);
    /**
     * Mix two audio buffer, two buff should have same len data
     * @param data1. in out data, restore the mixed result data
     * @param data2
     * @param rawDataCnt. sample count
     */
    void mixData(short *data1, short *data2, int rawDataCnt);
    void proccessSingleFile(list<shared_ptr<AudioFileMsg>> &lst, int& mixedLen);
    void preProcessAddMuteData(list<shared_ptr<AudioFileMsg>>::iterator &iter, int &mixedLen);
};

#endif
