//
// Created by Administrator on 2018/11/12.
//

#include "AudioResample.h"
#include "AILog.h"


int AudioResample::initResample(const char* outFilepath, int samples, int inChannels, int inSampleRate,
                  int inSampleFmt, int outChannnels, int outSampleRate, int outSampleFmt)
{
    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "aaaaaaaa");
    if (m_bRunning)
        return 0;
    LOGD("initResample");
    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "bbbbbbbbbb");
    m_bRunning = true;
    int dst_linesize;

    m_nSrcChannels = inChannels;
    m_nSrcSampleRate = inSampleRate;
    m_nSrcSampleFmt = inSampleFmt;

    m_nDstChannels = outChannnels;
    m_nDstSampleRate = outSampleRate;
    m_nDstSampleFmt = outSampleFmt;
    m_strOutFilepath = outFilepath;
    m_nbSamples = samples;

    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        LOGE("swr_alloc failed");
        return -1;
    }
    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "dddddddd");
    /* set options */
    int inChannalLayout, outChannelLayout;
    inChannalLayout = m_nSrcChannels == 1 ?  AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    outChannelLayout = m_nDstChannels == 1 ?  AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
    av_opt_set_int(swr_ctx, "in_channel_layout",    inChannalLayout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",       m_nSrcSampleRate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", (AVSampleFormat)m_nSrcSampleFmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout",    outChannelLayout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate",       m_nDstSampleRate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", (AVSampleFormat)m_nDstSampleFmt, 0);
    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%d %d %d %d %d %d samples:%d",
                        m_nSrcChannels, m_nSrcSampleRate, m_nSrcSampleFmt, m_nDstChannels,
                        m_nDstSampleRate, m_nDstSampleFmt, m_nbSamples);
    int ret = 0;
    if (ret = swr_init(swr_ctx) < 0) {
        AVERROR(ret);
        __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s %d", "error", ret);
        goto end;
    }

    int src_linesize;
    ret = av_samples_alloc_array_and_samples(&m_pSrcData, &src_linesize, m_nSrcChannels,
                                             m_nbSamples, (AVSampleFormat)m_nSrcSampleFmt, 0);
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "fffffff");
        LOGE("Could not allocate source samples");
        goto end;
    }
    LOGI("swr src buffersize:%d", ret);

    m_nMaxDstNumSamples = av_rescale_rnd(m_nbSamples, m_nDstSampleRate, m_nSrcSampleRate, AV_ROUND_UP);
    ret = av_samples_alloc_array_and_samples(&m_pDstData, &dst_linesize, m_nDstChannels,
                                             m_nMaxDstNumSamples, (AVSampleFormat)m_nDstSampleFmt, 0);
    LOGI("swr dst buffersize:%d dstSamples:%d", ret, m_nMaxDstNumSamples);

    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "zzzzzzzzzzz");
        LOGE("Could not allocate destination samples");
        goto end;
    }

    m_pFile = fopen(m_strOutFilepath.c_str(), "wb+");
    if (m_pFile == nullptr)
        goto end;

    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "lllllllllll");
    LOGI("create cycle buffer 1M");
    m_pCycleBuffer = make_shared<CCycleBuffer>(1<<20);
    m_thread = thread(&AudioResample::run, this);
    return ret;
end:
    m_bRunning = false;
    ret = -1;
    release();
    return ret;
}

void AudioResample::end()
{
    LOGD("end resample");
    m_bRunning = false;
    release();
}

void AudioResample::release()
{
    if (m_pFile)
        fclose(m_pFile);
    if (m_pSrcData)
        av_freep(&m_pSrcData[0]);
    av_freep(&m_pSrcData);

    if (m_pDstData)
        av_freep(&m_pDstData[0]);
    av_freep(&m_pDstData);
    swr_free(&swr_ctx);
}

int AudioResample::resample()
{
    int ret;
    int dst_linesize, dst_bufsize;
    LOGD("resample()");
    /* compute destination number of samples */
    int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, m_nSrcSampleRate) +
                                                m_nbSamples, m_nDstSampleRate, m_nSrcSampleRate, AV_ROUND_UP);
    LOGD("dst samples:%d maxSamples:%d", dst_nb_samples, m_nMaxDstNumSamples);
    if (dst_nb_samples > m_nMaxDstNumSamples) {
        av_freep(&m_pDstData[0]);
        LOGI("dst_nb_samples > m_nMaxDstNumSamples");
        ret = av_samples_alloc(m_pDstData, &dst_linesize, m_nDstChannels,
                               dst_nb_samples, (AVSampleFormat)m_nDstSampleFmt, 1);
        if (ret < 0)
            return -1;
        m_nMaxDstNumSamples = dst_nb_samples;
    }

    LOGD("dst samples:%d", dst_nb_samples);
    /* convert to destination format */
    ret = swr_convert(swr_ctx, m_pDstData, dst_nb_samples, (const uint8_t **)m_pSrcData, m_nbSamples);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        goto end;
    }
    dst_bufsize = av_samples_get_buffer_size(&dst_linesize, m_nDstChannels,
                                             ret, (AVSampleFormat)m_nDstSampleFmt, 1);
    if (dst_bufsize < 0) {
        LOGD("Could not get sample buffer size");
        goto end;
    }
    LOGD("write file dst_bufsize:%d", dst_bufsize);
    fwrite(m_pDstData[0], 1, dst_bufsize, m_pFile);
    return 0;
end:
    release();
    return -1;
}

void AudioResample::pushAudioData(uint8_t* data, int len)
{
    //LOGD("pushAudioData %p %d", data, len);
    m_pCycleBuffer->write(data, len);
}

void AudioResample::run() {
    LOGD("run()");
    while (m_bRunning) {
        if (m_pCycleBuffer->read(m_pSrcData[0], m_nbSamples * 2 * m_nSrcChannels) <= 0) {
            chrono::milliseconds sleepTime(20);
            this_thread::sleep_for(sleepTime);
            LOGD("sleep 20ms");
            continue;
        }

        resample();
    }
}