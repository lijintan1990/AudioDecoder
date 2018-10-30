package com.example.administrator.audiodecoder;

import java.util.List;

public class AudioMixer {


    public AudioMixer() {

    }
}
/*
interface AudioRawDataBack {
    void setAudioRawData(Byte[] data);
}

class AudioTransform implements AudioRawDataBack{

    @Override
    public void setAudioRawData(Byte[] data) {

    }

    void init() {
        AudioDecoder decoder = new AudioDecoder();
        AudioResample resample = new AudioResample();
        AudioMixer mixer = new AudioMixer();

        decoder.setNextObj(resample, this);
        resample.setNextObj(mixer, this);
    }
}

abstract class AbstractAudioWriteData {
    AbstractAudioWriteData nextObj;
    AudioRawDataBack callback;
    protected void setNextObj(AbstractAudioWriteData obj, AudioRawDataBack callback) {
        nextObj = obj;
        this.callback = callback;
    }

    public void writeData(Byte[] data, int level) {
        if (level > 3) {
            callback.setAudioRawData(data);
        } else {
            nextObj.writeData(data, level);
        }
    }
}

class AudioDecoder extends AbstractAudioWriteData {
    @Override
    public void writeData(Byte[] data, int level) {
        super.writeData(data, level);
    }
}

class AudioResample extends AbstractAudioWriteData {

}

class AudioMixer extends AbstractAudioWriteData {

}
*/