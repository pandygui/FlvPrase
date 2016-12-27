/*************************************************************************
    > File Name: flv_prase.c
    > Author: spygg
    > Mail: liushidc@163.com
    > Created Time: 2016年12月16日 星期五 11时20分35秒
 ************************************************************************/
//the environment is Ubuntu 16.04 and Cpu is Intel (Little endding I guess)
#include "flvprase.h"

bool found_duration = false;
double duration = 0;
int duration_pos = 0;

bool read_bytes(FILE *fp, byte *bdata, int ilen)
{
    if(fp == NULL)
        return false;

    int iret = 0;
    iret = fread(bdata, sizeof(byte), ilen, fp);
    if(iret != ilen || feof(fp) || ferror(fp))
        return false;

    return true;
}

unsigned short bytes2short(byte *data)
{
    unsigned int ret = (data[1]) | (data[0] << 8);

    return ret;
}

unsigned int bytes2int(byte *data)
{
    unsigned int ret = data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24);
    return ret;
}

unsigned int bytes2three_byte_int(byte *data)
{
    unsigned int ret = (data[2]) | (data[1] << 8) | (data[0] << 16);

    return ret;
}

bool bytes2bool(byte *data)
{
    return *data;
}

double bytes2double(byte *data)
{
    byte bytes[9] = {0};
    double value = 0;

    // //change the memory bytes order
    for(int i = 0; i < 8; i++)
    {
        bytes[7 - i] = *(data + i);
        //bytes[i] = *(data + i);
    }


    memcpy((byte*)&value, bytes, sizeof(double));

    //sscanf(bytes, "%lf", &value);

    //value = atof(bytes);
    if(found_duration)
    {
        duration += value;
        found_duration = false;
    }

    return value;
}

void double2bytes(double data, byte *out)
{
    byte bytes[8] = {0};
    memcpy(bytes, (byte*)&data, 8);

    //change the memory bytes order
    for(int i = 0; i < 8; i++)
    {
        out[7 - i] = *(bytes + i);
    }
}

FLV_HEADER prase_header(FILE *fp)
{
    FLV_HEADER flv_header;

    bool ret = read_bytes(fp, (byte*)&flv_header, sizeof(flv_header));
    if(!ret)
    {
        printf("errors when readfile\n");

        return flv_header;
    }


    //check is FLV formate
    if(strncmp((char*)(&flv_header.signature), "FLV", 3) != 0)
        return flv_header;

    printf("version \t\t--------------------> %d\n", flv_header.version);

    // check if have video
    if(flv_header.stream_info & 0x01)
        printf("has video \t\t--------------------> true\n");
    else
        printf("has video \t\t--------------------> false\n");


    if(flv_header.stream_info & (0x01 << 2))
        printf("has audio \t\t--------------------> true\n");
    else
        printf("has audio \t\t--------------------> false\n");

    printf("header size \t\t--------------------> %d\n", bytes2int(flv_header.header_offset));

    return flv_header;
}


bool read_flv_tag_header(FILE *fp, FLV_TAG_HEADER *flv_tag_header)
{
    if(fp == NULL)
        return false;

    int ret = fread(flv_tag_header, sizeof(byte), sizeof(FLV_TAG_HEADER), fp);
    if(ret != sizeof(FLV_TAG_HEADER) || feof(fp) || ferror(fp))
        return false;

    return true;
}


int prase_long_string(byte *data)
{
    int len = bytes2int(data);

    char *szData = malloc(len + 1);


    memset(szData, 0, len + 1);

    strncpy(szData, data + 4, len);


    printf("%s", szData);

    free(szData);

    return (len + 4);
}

int prase_string(byte *data)
{
    int len = bytes2short(data);

    char *szData = malloc(len + 1);


    memset(szData, 0, len + 1);

    strncpy(szData, data + 2, len);

    if(!found_duration && strcmp(szData, "duration") == 0)
    {
        found_duration = true;
    }

    printf("%s ", szData);

    free(szData);

    return (len + 2);
}


int prase_double(byte *data)
{
    int len = 0;
    double ret = bytes2double(data);

    printf(" %f", ret);
    return len + 8;
}

int prase_bool(byte *data)
{
    int len = 0;
    bool ret = bytes2bool(data);

    printf(" %d", ret);

    return len + 1;
}


int prase_object(byte *data)
{
    int len = 0;
    data += 0;

    while(1)
    {
        printf("\n");
        int ret = 0;
        ret = prase_string(data);
        data += ret;
        len += ret;

        printf(": ");
        ret = prase_sub_script_data(data);
        data += ret;
        len += ret;

        printf("\n");

        if(*data == 0x0 && *(data + 1) == 0x0 && *(data + 2) == 0x9)
        {

            len += 3;
            break;
        }


    }

    return len;
}

int prase_date(byte *data)
{
    return 10;
}

int prase_emc_array(byte *data)
{
    int len = 4;
    data += 4;

    while(1)
    {
        int ret = 0;

        ret = prase_string(data);
        data += ret;
        len += ret;
        
        printf(" \t\t--------------------> ");
        ret = prase_sub_script_data(data);
        data += ret;
        len += ret;
        printf("\n");

        if(*data == 0x0 && *(data + 1) == 0x0 && *(data + 2) == 0x9)
        {
            
            len += 3;
            break;
        }


    }

    //printf("ECMA len = %d\n", len);
    return len;
}

int prase_emc_array_strict(byte *data)
{
    int arr_len = bytes2int(data);
    int len = 4;
    data += 4;

    for(int i = 0; i < arr_len; i++)
    {
        int ret = 0;
        ret = prase_sub_script_data(data);


        data += ret;
        len += ret;
    }

    return len;
}

int prase_sub_script_data(byte *data)
{
    byte *p = data;
    int ret = 0;
    switch(*p)
    {
    case NUMBER_TYPE:
        p++;
        ret = prase_double(p);
        break;

    case BOOL_TYPE:
        p++;
        ret = prase_bool(data);
        break;

    case STRING_TYPE:
        p++;
        ret = prase_string(p);
        break;


    case OBJECT_TYPE:
        p++;
        ret = prase_object(p);
        break;

    case ECMA_ARRAY_TYPE:
        p++;
        ret = prase_emc_array(p);
        break;

    case DATE_TYPE:
        p++;
        ret = prase_date(p);
        break;

    case STRICT_ARRAY_TYPE:
        p++;
        ret = prase_emc_array_strict(p);
        break;


    case LONG_STRING_TYPE:
        p++;
        ret = prase_long_string(p);
        break;

    case MOVIE_CLIP_TYPE:
    case NULL_TYPE:
    case UNDEFINED_TYPE:
    case REFERENCE_TYPE:
        //do nothing here
        break;

    default:
        printf("error  %x, %x, %x, %x\n", *p, *(p + 1), *(p + 2), *(p + 3));
        break;
    }

    return (p + ret - data);
}


//type + data_len + data
void prase_script_data(byte *data, int data_len)
{
    for(int i = 0; i < data_len;)
    {
        int ret = prase_sub_script_data(data + i);
        i += ret;
        
        if(ret <= 0)
            break;
    }
}



void prase_audio_data(byte *data, int len)
{
    byte *p = data;
    AUDIO_INFO audio_type;
    memcpy(&audio_type, p, sizeof(audio_type));

    //printf("audio: %d, %d, %d, %d\n",  audio_type.audio_format, audio_type.sampling_rate, audio_type.sampling_len, audio_type.audio_type);
}

void prase_video_data(byte *data, int len)
{
	byte *p = data;

    VEDIO_INFO vedio_info;
    memcpy(&vedio_info, p, sizeof(vedio_info));

    //printf("vedio: %d, %d\n", vedio_info.encode, vedio_info.vedio_type);
}


bool prase_tag_data(FLV_TAG_HEADER tag_header, byte *data, int tag_data_len)
{
    byte tag_type = tag_header.type;

    if(tag_type == TYPE_SCRIPT)    //script data
    {
        //printf("script data\n");

        prase_script_data(data, tag_data_len);
        return false;
    }
    else if(tag_type == TYPE_AUDIO)
    {
        //printf("audio data\n");
        prase_audio_data(data, tag_data_len);
    }
    else if(tag_type == TYPE_VIDEO)
    {
        //printf("video data\n");
        prase_video_data(data, tag_data_len);

    }
    else
    {
        printf("error when prase tag data %x\n", tag_type);
    }

    return true;
}

// skip bytes for the file
bool skip(FILE *fp, int len)
{
    if(fp == NULL)
        return false;


    int ret = fseek(fp, len, SEEK_CUR);
    if(ret == 0)
        return true;

    return false;
}

void int2time_stamp(int data, byte *time_stamp, byte *time_stamp_ex)
{
    //byte bytes[3] = {0};
    *time_stamp_ex = data >> 24;
    data &= 0x00ffffff;

    time_stamp[0] = data >> 16;
    data &= 0x0000ffff;

    time_stamp[1] = data >> 8;
    data &= 0x000000ff;

    time_stamp[2] = data >> 0;
    data &= 0x00000000;
}


int time_stamp2int(byte time_stamp[3], byte time_stamp_ex)
{
    int ret = (time_stamp_ex << 24) | (time_stamp[2]) | (time_stamp[1] << 8) | (time_stamp[0] << 16);
    return ret;
}

bool prase_flv_file(char *file_name, FILE *fp_out, bool write_out, int *time_start_stamp)
{
    printf("############################\n");
    printf("file name: %s\n", file_name);

    FILE *fp = NULL;
    fp = fopen(file_name, "rb");

    int time_stamp = 0;

    if(fp)
    {
        FLV_HEADER header = prase_header(fp);

        if(fp_out && write_out)
        {
            fwrite(&header, sizeof(byte), sizeof(header), fp_out);
        }
        //deal flv body
        bool ret = false;
        //skip tag 0
        ret = skip(fp, 4);
        if(!ret)
        {
            return false;
        }

        if(fp_out && write_out)
        {
            skip(fp_out, 4);
        }



        while(1)
        {
            FLV_TAG_HEADER tag_header;

            //read tag header
            ret = read_flv_tag_header(fp, &tag_header);

            if(!ret)
                break;

            time_stamp = time_stamp2int(tag_header.time_stamp, tag_header.time_stamp_ex);
            time_stamp += *time_start_stamp;
            int2time_stamp(time_stamp, tag_header.time_stamp, &tag_header.time_stamp_ex);


            //time stamp
            //printf("after stamp:%d, %d, %d\n", time_stamp, *time_start_stamp, (tag_header.time_stamp_ex << 24) | bytes2three_byte_int(tag_header.time_stamp ));

            //get tag data length
            byte *tag_data = NULL;
            int data_len = bytes2three_byte_int(tag_header.data_len);
            tag_data = malloc(data_len);

            if(tag_data == NULL)
            {
                printf("malloc failed\n");
                return false;
            }

            //get tag data
            ret = read_bytes(fp, tag_data, data_len);
            if(!ret)
            {
                if(tag_data)
                {
                    free(tag_data);
                    tag_data = NULL;
                }
                break;
            }

            //prase tag data
            bool btype = prase_tag_data(tag_header, tag_data, data_len);


            if(fp_out)
            {
                if((!btype && write_out) || btype)
                {
                    fwrite(&tag_header, sizeof(byte), sizeof(tag_header), fp_out);
                    fwrite(tag_data, sizeof(byte), data_len, fp_out);
                }
            }

            if(tag_data)
            {
                free(tag_data);
                tag_data = NULL;
            }

            //read pre tag length( 4 bytes length)
            byte pre_tag_len[4] = {0};
            ret = read_bytes(fp, pre_tag_len, 4);

            if(!ret)
                break;

            if(fp_out)
            {
                if((!btype && write_out) || btype)
                    fwrite(pre_tag_len, sizeof(byte), 4, fp_out);
            }

        }

        fclose(fp);
    }
    else
    {
        printf("file open failed!\n");
        return false;
    }

    *time_start_stamp = time_stamp;

    printf("end file %s\n", file_name);

    return true;
}

bool merge_flv(int file_num, char *file_name[], char *file_out)
{
    printf("merge flv ....\n");

    char out_name[255] = {0};

    if(file_out == NULL)
    {
        strcpy(out_name, "out_by_c.flv");
    }
    else
        strcpy(out_name, file_out);   


    if(file_num <= 0)
    {
        return false;
    }
    else
    {
        FILE *fp_out = fopen(out_name, "w");

        int time_stamp_start = 0;

        for (int i = 0; i < file_num; ++i)
        {
            prase_flv_file(file_name[i], fp_out, !i, &time_stamp_start);
        }


        printf("duration: %f\n", duration);

        fclose(fp_out);
    }



    //the following stupid code's goal is change the vedio's time info 
    byte byte_read[1] = {0};
    FILE *fp_out = fopen(out_name, "r+");

    int file_pos = 0;
    while(1)
    {
        if(read_bytes(fp_out, byte_read, 1))
        {
            if(byte_read[0] == 'd')
            {
                if(read_bytes(fp_out, byte_read, 1))
                {
                    if(byte_read[0] == 'u')
                    {
                         if(read_bytes(fp_out, byte_read, 1))
                        {
                            if(byte_read[0] == 'r')
                            {
                                 if(read_bytes(fp_out, byte_read, 1))
                                {
                                    if(byte_read[0] == 'a')
                                    {
                                        
                                        if(read_bytes(fp_out, byte_read, 1))
                                        {
                                            if(byte_read[0] == 't')
                                            {
                                                
                                                if(read_bytes(fp_out, byte_read, 1))
                                                {
                                                    if(byte_read[0] == 'i')
                                                    {

                                                        if(read_bytes(fp_out, byte_read, 1))
                                                        {
                                                            if(byte_read[0] == 'o')
                                                            {
                        
                                                                if(read_bytes(fp_out, byte_read, 1))
                                                                {
                                                                    if(byte_read[0] == 'n')
                                                                    {
                                                                        file_pos = ftell(fp_out);
                                                                        printf("ftell %ld\n", ftell(fp_out));
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }  
        }
        else
            break;
    }


    //revise time info
    fprintf(fp_out, "%f\n", duration);
    
    fclose(fp_out);

    return true;
}


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("useage: flv_prase filename.flv %d\n", argc);
        return -1;
    }

    merge_flv(argc - 1, &argv[1], NULL);

    return 0;
}


