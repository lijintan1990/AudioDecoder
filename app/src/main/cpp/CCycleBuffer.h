//
// Created by Administrator on 2018/11/7.
//

#ifndef AUDIODECODER_CCYCLEBUFFER_H
#define AUDIODECODER_CCYCLEBUFFER_H


//环形缓冲区头文件,预留一个空间大小
#include <memory>
#include <mutex>
using namespace std;

class CCycleBuffer
{
public:
    CCycleBuffer(int size);
    virtual~CCycleBuffer();
    void write(uint8_t* buf, int len);
    int read(uint8_t* buf, int len);
	void reset();

private:
    bool isFull();
    bool isEmpty();

private:
    bool m_bEmpty, m_bFull;
    shared_ptr<uint8_t> m_pBuf;
    int m_nBufSize;
    int m_nReadPos;
    int m_nWritePos;
    mutex m_mutex;
};


#endif //AUDIODECODER_CCYCLEBUFFER_H
