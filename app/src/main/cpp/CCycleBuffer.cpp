//
// Created by Administrator on 2018/11/7.
//

#include "CCycleBuffer.h"
#include "AILog.h"
#include <assert.h>
#include <memory.h>

CCycleBuffer::CCycleBuffer(int size)
{
    __android_log_print(ANDROID_LOG_ERROR, "AINative", "%s", "cccccccccc");
    m_nBufSize = size;
    m_nReadPos = 0;
    m_nWritePos = 0;
    m_pBuf = shared_ptr<uint8_t>(new uint8_t[m_nBufSize], [](uint8_t *p){delete []p;});
    LOGD("m_nBufSize:%d", m_nBufSize);
}

CCycleBuffer::~CCycleBuffer()
{
}

void CCycleBuffer::write(uint8_t* buf, int len)
{
    if (len <= 0 || !m_pBuf || isFull()) {
        LOGD("len:%d %p isFull:%d", len, m_pBuf.get(), isFull());
        return;
    }

    lock_guard<mutex> lock(m_mutex);
    uint8_t *pBuf = m_pBuf.get();
    // ------------w----------r--------
    if (m_nReadPos > m_nWritePos) {
        int remaindSize = m_nReadPos - m_nWritePos - 1;
        if (remaindSize >= len) {
            memcpy(pBuf + m_nWritePos, buf, len);
            m_nWritePos += len;
        } else {
            memcpy(pBuf + m_nWritePos, buf, remaindSize);
            m_nWritePos += remaindSize;
            LOGE("drop audio len:%d", len - remaindSize);
        }
    } else {
        int remaindSize = m_nBufSize - m_nWritePos + m_nReadPos - 1;
        // have enougth buff
        if (remaindSize >= len) {
            if (m_nBufSize - m_nWritePos >= len) {
                memcpy(pBuf+m_nWritePos, buf, len);
                m_nWritePos = (m_nWritePos + len) % m_nBufSize;
            } else {
                int firstCopySize = m_nBufSize - m_nWritePos;
                int secondCopySize = len - firstCopySize;
                memcpy(pBuf+m_nWritePos, buf, firstCopySize);
                m_nWritePos = 0;
                memcpy(pBuf, buf+firstCopySize, secondCopySize);
                m_nWritePos += secondCopySize;
            }
        } else {
            int firstCopySize = m_nBufSize - m_nWritePos;
            int secondCopySize = m_nReadPos - 1;
            if (m_nReadPos == 0) {
                firstCopySize -= 1; //必须预留一个空间,这里就是尾部预留一个空间
                memcpy(pBuf+m_nWritePos, buf, firstCopySize);
                m_nWritePos += firstCopySize;
            } else {
                memcpy(pBuf+m_nWritePos, buf, firstCopySize);
                //从起始位置继续写入
                m_nWritePos = 0;
                if (secondCopySize > 0) {
                    memcpy(pBuf, buf + firstCopySize, secondCopySize);
                }
                LOGE("drop audio len:%d", len - remaindSize);
            }
        }
    }

    LOGD("rPos:%d wPos:%d", m_nReadPos, m_nWritePos);
}
//#define NEED_READ
int CCycleBuffer::read(uint8_t* buf, int len)
{
    if (len <= 0 || !m_pBuf || isEmpty()) {
        LOGW("len:%d m_pBuf:%p %d", len, m_pBuf.get(), isEmpty());
        return -1;
    }

    lock_guard<mutex> lock(m_mutex);
    uint8_t *pBuf = m_pBuf.get();
    // ----------w-----------------r---------
    // w---------------r-------------
    if (m_nReadPos > m_nWritePos) {

        if (m_nBufSize - m_nReadPos >= len) {
            memcpy(buf, pBuf+m_nReadPos, len);
            m_nReadPos = (m_nReadPos + len) % m_nBufSize;
        } else {
            int firstCopySize = m_nBufSize - m_nReadPos;
            memcpy(buf, pBuf+m_nReadPos, firstCopySize);
            m_nReadPos = 0;
            //继续从起始位置读取
			if (m_nWritePos < len - firstCopySize)
			{
				// do not have enougth data

#ifdef NO_NEED_READ_FULL
				memcpy(buf + firstCopySize, pBuf, m_nWritePos);
				m_nReadPos = m_nWritePos;
				return firstCopySize + m_nWritePos;
#else
				return -1;
#endif
			}
			else {
				// have engouth data
				memcpy(buf + firstCopySize, pBuf, len - firstCopySize);
				m_nReadPos += len - firstCopySize;
			}
        }
    } else {
        // --------------r-------------------w----
        int remaindSize = m_nWritePos - m_nReadPos;
        if (remaindSize >= len) {
            memcpy(buf, pBuf+m_nReadPos, len);
            m_nReadPos += len;
        }else {
#if NO_NEED_READ_FULL
            memcpy(buf, pBuf+m_nReadPos, remaindSize);
            m_nReadPos += remaindSize;
            printf("not get enougth data");
            return remaindSize;
#else
            return -1;
#endif
        }
    }

	return len;
}

void CCycleBuffer::reset()
{
	lock_guard<mutex>lock(m_mutex);
	m_nReadPos = m_nWritePos = 0;
}

bool CCycleBuffer::isEmpty()
{
    bool ret = false;
    lock_guard<mutex> lock(m_mutex);
    if (m_nWritePos == m_nReadPos)
        ret = true;
    return ret;
}

bool CCycleBuffer::isFull()
{
    bool ret = false;
    lock_guard<mutex> lock(m_mutex);

    if ((m_nWritePos + 1) % m_nBufSize == m_nReadPos)
        ret = true;

    return ret;
}