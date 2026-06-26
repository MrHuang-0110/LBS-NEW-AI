#ifndef __MUSIC_H
#define __MUSIC_H
#include "sys.h"

enum
{ 
  NEW_MUSIC_DATA,
	OLD_MUSIC_DATA,
	STOP_MUSIC,
};
typedef struct {
    char ChunkID[4];
    uint32_t ChunkSize;
    char Format[4];
    char Subchunk1ID[4];
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    char Subchunk2ID[4];
    uint32_t Subchunk2Size;
}WAVheader;

typedef struct
{
   char *Playerpath;
   uint8_t *data;
   
   float volumento;
}_IO_MUSIC;

typedef struct
{ 
   uint8_t type;
	 uint8_t *music_data;
	 uint32_t data_length;
	 float sduty;
	 bool is_playing;
}MusicCommand_t;

typedef struct {
    uint8_t *data;
    uint32_t length;
    uint32_t position;
	  float sduty;
} MusicState_t;

void musicInfoInit(void);
void player_music(char *wav_path);
void player_button_music(void);
void setmusicvolumento(float volumento);
void musicStop(void);
void fullmusicpath(char *musicpath);
void playerMusicFram(uint8_t *pData,uint16_t length);
void xMusicSendQueueCmd(MusicCommand_t *cmd);
void delet_music_player(void);
void create_music_play_task(void);
#endif
