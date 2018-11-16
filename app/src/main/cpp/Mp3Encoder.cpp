#include "lame.h"
#include "Mp3Encoder.h"


Mp3Lame::Mp3Lame()
{

}

Mp3Lame::~Mp3Lame()
{

}

void Mp3Lame::init(int inSamplerate, int inChannel, int outSamplerate, int outBitrate, int quality) {
	if (lame != NULL) {
		lame_close(lame);
		lame = NULL;
	}
	lame = lame_init();
	lame_set_in_samplerate(lame, inSamplerate);
	lame_set_num_channels(lame, inChannel);//输入流的声道
	lame_set_out_samplerate(lame, outSamplerate);
	lame_set_brate(lame, outBitrate);
	lame_set_quality(lame, quality);
	lame_init_params(lame);
}

int Mp3Lame::encode(short* buffer_l, short* buffer_r, int samples, uint8_t* mp3buf, int mp3buf_size)
{
	int result = lame_encode_buffer(lame, buffer_l, buffer_r,
			samples, mp3buf, mp3buf_size);

	return result;
}

int Mp3Lame::flush(uint8_t *mp3buf, int mp3buf_size) {
	int result = lame_encode_flush(lame, mp3buf, mp3buf_size);
	return result;
}

void Mp3Lame::close()
{
	lame_close(lame);
	lame = NULL;
}
