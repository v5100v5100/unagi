#ifndef _WAVEFORM_DOZEU_H_
#define _WAVEFORM_DOZEU_H_
enum{
	WAVE_NUM = 4,
	INIT_SIZE = 8,
	WAVE_SIZE = 32,
	FLOW_SIZE = 9
};
struct gpif_data{
	u8 wave[WAVE_NUM][WAVE_SIZE];
	u8 flow[WAVE_NUM][FLOW_SIZE];
};
extern const u8 WAVEFORM_INIT[INIT_SIZE];
extern const struct gpif_data *const WAVEFORM_DATA;
#endif
