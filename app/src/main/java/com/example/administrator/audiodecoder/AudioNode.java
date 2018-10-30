package com.example.administrator.audiodecoder;

public class AudioNode {
    public static final int BGM_TYPE = 0;
    public static final int VOICE_TYPE = 1;

    public String mFilePath;
    public int mFileType;

    AudioNode(int type, String filepath) {
        mFilePath = filepath;
        mFileType = type;
    }
}
