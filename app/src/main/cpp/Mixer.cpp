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

void Mixer::mergeMusic()
{
    LOGD("mixFileRun");
    int oncelen = 2048;
    uint8_t data[oncelen] = {0};
	char *mainFilePath = "/sdcard/main.pcm";
    FILE *mainFile = fopen(mainFilePath, "wb+");
    //便于简化流程，先把音乐结合成一个文件，然后在进行混音
    for (auto iter : m_musicLst) {
        FILE *oneFile = fopen(iter->filename)
        if (oneFile == nullptr) {
            LOGE("open %s failed", iter->filepath.c_str());
            continue;
        }
		
        LOGE("merge %s to main file", iter->filepath.c_str());
        int mutelen = rescaleAudioMillsecToLen(iter->startTime);

        while (mutelen > oncelen) {
            fwrite(data, 1, oncelen, mainFile);
            mutelen -= oncelen;
        }
        if (mutelen > 0) {
            fwrite(data, 1, mutelen, mainFile);
        }

        int retLen;
        
        do {
            ret = fread(data, 1, oncelen, oneFile);
            if (ret > 0) {
                fwrite(data, 1, ret, mainFile);
            } else {
                LOGE("read bread");
                break;
            }
        } while(!feof(oneFile));
        fclose(oneFile);
    }

	auto ptr = m_musicLst->rbegin();
	int remainTime = m_nTotalDuration - (*ptr)->startTime + (*ptr)->duration;

	while (remainTime > oncelen) {
		fwrite(data, 1, oncelen, mainFile);
		remainTime -= oncelen;
	}
	if (remainTime > 0) {
		fwrite(data, 1, mutelen, mainFile);
	}

	fclose(mainFile);
}

void Mixer::mergeAll()
{
	int ret;
	int onceHandleLen = 2048;
	uint8_t data[onceHandleLen] = { 0 };
	uint8_t voiceData[onceHandleLen] = { 0 };
	char *mainFilePath = "/sdcard/main.pcm";
	char *targetFile = "/sdcard/target.pcm";
	FILE* musicFile = fopen(mainFilePath, "rb+");
	int mergedLen = 0;

	list<shared_ptr<AudioFileMsg>>::iterator voiceIter = m_voiceLst.begin();
	for (auto sp : m_voiceLst) 
	{
		LOGI("mix %s", sp->filepath.c_str());
		FILE* voiceFile = fopen(sp->filepath.c_str(), "rb+");
		if (voiceFile == nullptr) {
			LOGE("open %s failed", sp->filepath.c_str());
			continue;
		}

		int startLen = rescaleAudioMillsecToLen(sp->startTime);
		if (startLen < mergedLen)
		{
			LOGE("startLen:%d < mergedLen:%d err", startLen, mergedLen);
			fclose(voiceFile);
			continue;
		}

		// 不用混音的数据
		int noMixLen = startLen - mergedLen;
		while (noMixLen > onceHandleLen)
		{
			ret = fread(data, 1, onceHandleLen, musicFile);
			if (ret < 0) {
				LOGE("fread failed %d", ret);
				break;
			}
			deliverOutRawData(data, onceHandleLen);
			noMixLen -= onceHandleLen;
			mergedLen += onceHandleLen;
		}

		// 混音
		do 
		{
			memset(voiceData, 0, onceHandleLen);
			//不需要额外处理尾部数据，没有必要
			ret = fread(voiceData, 1, onceHandleLen, voiceFile);
			if (ret < 0) {
				LOGE("fread failed %d", ret);
			}

			ret = fread(data, 1, onceHandleLen, musicFile);
			if (ret < 0) {
				LOGE("fread failed %d", ret);
				break;
			}

			mixData(data, voiceData, onceHandleLen / 2);
			deliverOutRawData(data, rescaleAudioLenToMillsec(mergedLen));
			mergedLen += onceHandleLen;
		} while (!feof(voiceFile));

		fclose(voiceFile);
	}

	do
	{
		memset(data, 0, onceHandleLen);
		ret = fread(data, 1, onceHandleLen, musicFile);
		if (ret < 0) {
			LOGE("fread failed %d", ret);
			break;
		}

		deliverOutRawData(data, rescaleAudioLenToMillsec(mergedLen));
		mergedLen += onceHandleLen;
	} while (!feof(musicFile));
	fclose(musicFile);
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