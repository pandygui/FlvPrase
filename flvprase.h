#include<assert.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>


typedef unsigned char byte;
typedef unsigned char bool;

#define true 1
#define false 0


//flv file header
typedef struct _FLV_HEADER
{
    byte signature[3];
    byte version;
    byte stream_info;		//last bit have video, last bit from three represt audio
    byte header_offset[4];		//often is 9

}FLV_HEADER;

// flv tag header
typedef struct _FLV_TAG_HEADER
{
    byte type;
    byte data_len[3];
    byte time_stamp[3];
    byte time_stamp_ex;
    byte stream_id[3];
}FLV_TAG_HEADER;


typedef struct _FLV_TAG
{
    FLV_TAG_HEADER tag_header;
    byte *tag_data;
}FLV_TAG;

// notice the bits order is reverse
// my pc is intel' CPU(little endding i guess)
typedef struct  _AUDIO_INFO
{
    byte audio_type:1;

    byte sampling_len:1;

    byte sampling_rate:2;   // 0 = 5.5-kHz
                            // 1 = 11-kHz
                            // 2 bits
                            // 2 = 22-kHz
                            // 3 = 44-kHz
                            // 对于 AAC 总是 3
    
    byte audio_format:4;    // 0 = Linear PCM, platform endian
                            // 1 = ADPCM
                            // 2 = MP3
                            // 3 = Linear PCM, little endian
                            // 4 = Nellymoser 16-kHz mono
                            // 5 = Nellymoser 8-kHz mono
                            // 6 = Nellymoser
                            // 4 bits
                            // 7 = G.711 A-law logarithmic PCM
                            // 8 = G.711 mu-law logarithmic PCM
                            // 9 = reserved
                            // 10 = AAC
                            // 11 = Speex
                            // 14 = MP3 8-Khz
                            // 15 = Device-specific sound

}AUDIO_INFO;

typedef struct  _VEDIO_INFO
{
    byte encode:4;      // 1: JPEG,
                        // 2: Sorenson H.263
                        // 3: Screen video
                        // 4 bits
                        // 4: On2 VP6
                        // 5: On2 VP6 with alpha channel
                        // 6: Screen video version 2
                        // 7: AVC

    byte vedio_type:4;  // 1: keyframe (for AVC, a seekable frame)
                        // 2: inter frame (for AVC, a non-seekable frame)
                        // 4 bits
                        // 3: disposable inter frame (H.263 only)
                        // 4: generated keyframe (reserved for server use only)
                        // 5: video info/command frame

}VEDIO_INFO;

enum TAG_TYPE
{
    TYPE_AUDIO = 8,
    TYPE_VIDEO = 9,
    TYPE_SCRIPT = 18,
};

enum SCRIPT_TYPE
{
    NUMBER_TYPE = 0,
    BOOL_TYPE,
    STRING_TYPE,
    OBJECT_TYPE,
    MOVIE_CLIP_TYPE,
    NULL_TYPE,
    UNDEFINED_TYPE,
    REFERENCE_TYPE,
    ECMA_ARRAY_TYPE = 8,
    STRICT_ARRAY_TYPE = 10,
    DATE_TYPE = 11,
    LONG_STRING_TYPE = 12
};



bool read_bytes(FILE *fp, byte *bdata, int ilen);
unsigned short bytes2short(byte *data);
unsigned int bytes2int(byte *data);
unsigned int bytes2three_byte_int(byte *data);
bool bytes2bool(byte *data);
double bytes2double(byte *data);
void int2three_bytes(int data, byte *result);

FLV_HEADER prase_header(FILE *fp);

bool read_flv_tag_header(FILE *fp, FLV_TAG_HEADER *flv_tag_header);
int prase_long_string(byte *data);
int prase_string(byte *data);
int prase_double(byte *data);
int prase_bool(byte *data);
int prase_object(byte *data);
int prase_date(byte *data);
int prase_emc_array(byte *data);
int prase_emc_array_strict(byte *data);
int prase_sub_script_data(byte *data);
void prase_script_data(byte *data, int data_len);
bool prase_tag_data(FLV_TAG_HEADER tag_header, byte *data, int tag_data_len);
void prase_audio_data(byte *data, int len);
void prase_video_data(byte *data, int len);
bool skip(FILE *fp, int len);
bool prase_flv_file(char *file_name, FILE *fp_out, bool write_out, int *time_stamp_start);
