//
// Created by Administrator on 2018/11/15.
//

#ifndef AUDIODECODER_TRANSCODE_H
#define AUDIODECODER_TRANSCODE_H
#include <string>
/**
 * pcm编码成mp3
 * pcm 混音流程控制
 */
class Transcode {
public:
    Transcode();
    virtual ~Transcode();

    void encodePcmToMp3(std::string &infile, std::string outfile);

};

#endif //AUDIODECODER_TRANSCODE_H
