//
// Created by Administrator on 2018/11/15.
//

#include "Transcode.h"
#include <memory>
#include "Mp3Encoder.h"
#include "AILog.h"

using namespace std;
#define _SAMPLE_RATE 44100
#define _AUDIO_CHANNELS 1

Transcode::Transcode() {

}

Transcode::~Transcode() {

}

void Transcode::encodePcmToMp3(std::string &infile, std::string outfile) {
    int ret;
    shared_ptr<Mp3Lame> mp3lame = make_shared<Mp3Lame>();
    //int inSamplerate, int inChannel, int outSamplerate, int outBitrate, int quality
    mp3lame->init(_SAMPLE_RATE, _AUDIO_CHANNELS, _SAMPLE_RATE, 128, 5);
    shared_ptr<short> inBuf(new short[1<<14], [](short *p){delete [] p;});
    shared_ptr<uint8_t> outBuf(new uint8_t[1<<13], [](short*p) {delete[] p;});
    FILE* pInfile = fopen(infile.c_str(), "rb+");
    if (!pInfile) {
        LOGE("fopen failed");
        return;
    }
    FILE* pOutFile = fopen(outfile.c_str(), "wb+");
    if (!pOutFile) {
        LOGE("fopen failed");
        return;
    }

    do {
        ret = fread(inBuf.get(), 1, 1<<15, pInfile);
        if (ret > 0) {
            ret = mp3lame->encode(inBuf.get(), nullptr, ret/2, outBuf.get(), 1<<13);
            fwrite(outBuf.get(), 1, ret, pOutFile);
        }
    }while (!feof(pInfile));

    ret = mp3lame->flush(outBuf.get(), 0);
    fwrite(outBuf.get(), 1, ret, pOutFile);
    mp3lame->close();
}

