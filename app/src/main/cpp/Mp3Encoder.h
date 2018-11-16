#ifndef MP3_ENCODER_H
#define MP3_ENCODER_H
#include "lame.h"
#include <string>

class Mp3Lame {
    lame_global_flags *lame = NULL;
public:
    Mp3Lame();
    virtual ~Mp3Lame();
    void init(int inSamplerate, int inChannel, int outSamplerate, int outBitrate, int quality);
    int encode(short* buffer_l, short* buffer_r, int samples, uint8_t* mp3buf, int mp3buf_size);
    int flush(uint8_t *mp3buf, int mp3buf_size);
    void close();
};

#endif