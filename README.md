# FlvPrase
a flv file contact by c language
也说FLV格式分析（C语言完整版）

最近想写一个在线视频播放软件，经研究得知某视频网站会将一个视频成很多段来投放从而方便在线缓冲，以及
加Ad之类的目的。下载后大概如下图所示（星哥的九品芝麻官），于是就想合并这些视频，所以就研究了下FLV的格式，然后用C语言写了一个很丑陋的版本。





















本教程暂且分成两部分：FLV格式分析和FLV文件的合并，网上有很多类似的教程，比如：FLV文件格式解析已经给出了大概的说明，本教程在此基础上进一步分析，给出一个完整可运行的版本。

一.FLV格式分析
FLV是Adobe公司制定的格式用来取代Swf格式。所以最权威的文档肯定是要去其官网上下载FLV的手册了，不过鉴于是英文版本，本着一事不烦二主的原则这里在翻译一下。希望读本教程也能知道怎么去解析一
个FLV文件。

一个FLV文件分成文件头（FLVheader）和文件内容（FLVfile body）两部分。
在分析之前先说一下FLV的数据类型：UI8,UI16,UI24分别代表1个,2个3个字节的无符号整形数据。并且FLV文件采用大端格式进行存储：如十进制的300为0x010x2C。（下面这句话引自官方文档video_file_format_spec_v10.pdf）
FLV files store multibyteintegers in big-endian byte order; as a UI16 in FLV file format, thebyte sequence that represents the number 300 is 0x01 0x2C. Also, FLVfiles use a 3-byte integer type: a UI24 represents an unsigned 24-bitinteger.

1.文件头（FLVheader）

Field
Type
Comment
Signature（签名）
UI8
恒为‘F’（0x46）
Signature
UI8
恒为‘L’（0x4c)
Signature
UI8
恒为‘V’ (0x56)
Version（版本）
UI8
FLV文件版本一般为0x1（还没见过其他值的）
TypeFlagsReserved
UB[5]
5个比特的保留位恒为0
TypeFlagsAudio（视频标志）
UB[1]
该比特为1时代表有视频
TypeFlagsReserved
UB[1]
1个比特的保留位恒为0
TypeFlagsVideo（音频标志）
UB[1]
1个比特为1时代表有音频
DataOffset（文件头大小）
UI32
为9 （该表格中长度之和）


如上图中的被选中部分。所以可以定义文件头数据结构如下：
typedef struct _FLV_HEADER
{
    byte signature[3];
    byte version;
    byte stream_info;   //last bit have video, last bit from three represt audio
    byte header_offset[4];              //often is 9

}FLV_HEADER;

//解析函数
bool prase_header(FLV_HEADER *flv_header)
{
    //check is FLV formate
    if(strncmp((char*)(&flv_header->signature), "FLV", 3) != 0)
        return false;

    printf("version: %d\n", flv_header->version);

    // check if have video
    if(flv_header->stream_info & 0x01)
        printf("has video: true\n");
    else
        printf("has video: false\n");


    if(flv_header->stream_info & (0x01 << 2))
        printf("has audio: true\n");
    else
        printf("has audio: false\n");

    printf("header size: %d\n", bytes2int(flv_header->header_offset));

    return true;
}

2.文件内容（FLVfile body）

文件头后紧跟的是文件内容，如下图所示（无耻的剽窃自别人的博客）



















由上图可以看出，body部分由一个个Tag组成，每个Tag的下面有一块4bytes的空间用来记录这个tag的⻓度,这个后置用于逆向。当然第一个tag的长度肯定是0了（PreviosTagSize0为4个字节的0)
2.1 Tag 解析
每一个tag也是由两部分组成：tag头和tag数据区，好了至此FLV格式分析结束，嘿嘿（裤子都脱了你就给看这个）。其实只是想强调下每个tag都是由 头+数据 组成。如下表所示，很明显tag头为11个字节。

2.1.1 tag头
tag头为下表中的前11个字节，剩下的为tag数据区（具体分析见2.1.2）

名称
⻓度
介绍
TagType（tag类型）
UI8
8: audio(音频）
9: video（视频）
18: script data（脚本数据）
all others: reserved（其他保留）
DataSize（tag数据区大小）
UI24
tag的数据区大小以字节为单位，注意字节序
Timestamp（时间戳）

本tag相对于FLV文件第一个tag的时间，以毫秒为单位，当然第一个tag的时间戳肯定为0
TimestampExtended（时间戳扩展）
UI8
将时间戳扩展至32位，该字节代表高8位
StreamID（信息流ID)
UI24
恒为0
Data（tag数据区）
If TagType == 8
AUDIODATA
If TagType == 9
VIDEODATA
If TagType == 18
SCRIPTDATAOBJECT
由第一个字节的tag类型决定：
为8时代表音频
为9时代表视频
为18时代表脚本数据

2.1.2 tag数据
tag数据分为：脚本数据，音频数据以及视频数据，因为脚本数据比较复杂先详细分析脚本数据。

2.1.2.1脚本数据
一般来说脚本数据都是第一帧，所以先分析脚本数据吧，这个是FLV文件中最复杂的部分（音频和视频数据比较简单）
脚本数据包含了一个FLV文件的信息数据如：分辨率，视频长度，码率，采样率等信息...
脚本数据的数据类型有：
0 = Number type (double型）
1 = Boolean type（bool型）
2 = String type（字符串型）
3 = Object type
4 = MovieClip type
5 = Null type
6 = Undefined type
7 = Reference type
8 = ECMA array type
10 = Strict array type
11 = Date type（时间类型）
12 = Long string type（长字符串）

所有的数据可由公式来表达，  公式 =  数据类型 + 数据长度（根据类型决定） + 数据  三部分组成
注意：公式中可以确定长度的数据长度就会省略（bool , double），有些类型确定的类型就省略（下面介绍）看来真是抠门到家了，为了弄清这点我拿了一个16禁止编辑器（非常推荐QtCreator或者Vc 编辑器，可以直接打开）一个字节一个字节对比才得出这点。
连官方文档都没有细说，如果本人理解有误请在留言中指出！！！

上面的数据类型，我分为两类：
1.简单类型包括：
double类型 （数据长度为8个字节，省略）
bool类型（数据长度为1个字节，省略）
string 类型（数据长度为2个字节，然后是字符串数据）
long string类型（数据长度为4个字节，然后是字符串数据）

2.复杂类型：复杂类型为简单类型的组合
Strict array type（严格数组，翻译的不准，嘿嘿）：数组的长度一定，根据数组长度可以知道该数组有多少项（如a[10]表示有10个数据项），项数为数据类型后的4个字节，数组中的每一项也是用上面的公式表示

Object type（object类型，靠词穷了）：为一个数据字典（Python的说法）：key + value
其中key一定为string类型（所以代表string类型的2省略），value为随意类型的数据，见公式，需要说明的是可以该项可以嵌套，必须以00 00 09结尾

ECMA array type（Ecma数组）：
为一个数据字典（Python的说法）：key + value其中key一定为string类型（所以代表string类型的2省略），value为随意类型的数据的数组。该数组也装模作样的用4个字节来表示该数组的大小，但是然并卵，你并不能通过这个值来做什么，该项以00 00 09结尾，该项也可以嵌套
下面贴出FLV文件的一部分，包括FLV头和一个脚本数据



46 4c 56 01 05 00 00 00 09（FLV文件头）
00 00 00 00 （第一帧数据的长度）
12 00 04 f0 00 00 00 00 00 00 00（tag头，可以知道类型为18脚本数据，长度为 0x 00 04 f0 = 1264字节）
02 00 0a 6f 6e 4d 65 74 61 44 61 74 61(类型为字符串，长度00 0a = 10个内容为onMetaData）
08 00 00 00 0b （为ecma array类型，项数为不靠谱的11）

00 0f 6d 65 74 61 64 61 74 61 63 72 65 61 74 6f 72（key:string类型，类型字节省略, 长度为00 0f ,数据为metadatacreator）
02 00 21 6d 6f 64 69 66 69 65 64 20 62 79 20 79 6f 75 6b 75 2e 63 6f 6d 20 69 6e 20 32 30 31 31 31 32 30 32（value:类型为02字符串，长度为00 21,数据为modified by youku.com in 20111202（00 21后面的33个字节）

00 0c 68 61 73 4b 65 79 66 72 61 6d 65 73（key:string类型，长度00 0c，数据hasKeyframes）
01 01（value:类型为01 bool类型，数值为01）

00 08 68 61 73 56 69 64 65 6f 01 01 00 08 68 61 73 41 75 64 69 6f 01 01 00 0b 68 61 73 4d 65 74 61 64 61 74 61 01 01 00 05 77 69 64 74 68 00 40 91 c0 00 00 00 00 00 00 06 68 65 69 67 68 74 00 40 83 10 00 00 00 00 00 00 09 66 72 61 6d 65 72 61 74 65 00 40 37 f9 dc b5 11 22 87 00 0f 61 75 64 69 6f 73 61 6d 70 6c 65 72 61 74 65 00 40 d5 88 80 00 00 00 00 00 08 64 75 72 61 74 69 6f 6e 00 40 67 7b 56 b2 db d1 94 00 09 6b 65 79 66 72 61 6d 65 73

03（object类型）
00 0d 66 69 6c 65 70 6f 73 69 74 69 6f 6e 73（key:类型为string类型（省略）长度00 0d，值：filepositions）
0a 00 00 00 37 （value:类型为Strict array，数组的项数为00 00 00 37 = 55项）
00 40 94 30 00 00 00 00 00 （类型00为double类型，所以长度省略，接下来的8个字节代表值 1292.000000）
00 40 95 a8 00 00 00 00 00（类型00为double类型，所以长度省略，接下来的8个字节代表值 1386.000000）
00 40 f9 63 30 00 00 00 00 00 41 10 83 1c 00 00 00 00 00 41 1c 18 38 00 00 00 00 00 41 21 83 9e 00 00 00 00 00 41 29 29 8e 00 00 00 00 00 41 34 d7 04 00 00 00 00 00 41 3b 19 dd 00 00 00 00 00 41 3d 71 aa 00 00 00 00 00 41 3e ef 0b 00 00 00 00 00 41 47 78 7f 00 00 00 00 00 41 4e f8 c0 80 00 00 00 00 41 51 cf 99 80 00 00 00 00 41 55 70 e2 c0 00 00 00 00 41 58 fb fa 80 00 00 00 00 41 5c 9f ea 80 00 00 00 00 41 5d b7 d8 00 00 00 00 00 41 5e b8 b9 80 00 00 00 00 41 60 5e e9 80 00 00 00 00 41 60 d6 48 40 00 00 00 00 41 61 29 4d c0 00 00 00 00 41 61 86 44 60 00 00 00 00 41 62 0e 49 00 00 00 00 00 41 62 6f b6 60 00 00 00 00 41 62 e1 c3 00 00 00 00 00 41 63 68 59 40 00 00 00 00 41 64 1a 54 a0 00 00 00 00 41 64 d0 c9 60 00 00 00 00 41 66 0a 50 a0 00 00 00 00 41 66 98 d6 20 00 00 00 00 41 66 c1 67 e0 00 00 00 00 41 66 cd 8e 20 00 00 00 00 41 66 e4 b3 c0 00 00 00 00 41 67 3e 92 80 00 00 00 00 41 68 1a ed 00 00 00 00 00 41 69 25 f9 00 00 00 00 00 41 69 9a 09 e0 00 00 00 00 41 69 e7 0d 40 00 00 00 00 41 6a 02 ef 20 00 00 00 00 41 6a 9b 54 20 00 00 00 00 41 6c 4c db 40 00 00 00 00 41 6d 08 bf 20 00 00 00 00 41 6e 39 83 20 00 00 00 00 41 70 83 14 f0 00 00 00 00 41 71 04 77 e0 00 00 00 00 41 71 c3 82 60 00 00 00 00 41 72 77 6a 40 00 00 00 00 41 72 c5 e1 40 00 00 00 00 41 73 6a 92 30 00 00 00 00 41 73 eb cf 20 00 00 00 00 41 74 25 d7 70 00 00 00 00 41 74 52 66 00 00 00 00 00 41 74 97 38 50 00 00 00 00 41 75 41 a2 40 00 00 00 00 05 74 69 6d 65 73 0a 00 00 00 37 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 40 14 05 1e b8 51 eb 85 00 40 23 84 fd f3 b6 45 a1 00 40 2f 88 10 62 4d d2 f1 00 40 33 3a 40 2b b0 cf 87 00 40 36 c5 d2 f1 a9 fb e7 00 40 3c c7 5c 28 f5 c2 8f 00 40 41 64 72 b0 20 c4 9b 00 40 42 84 bc 6a 7e f9 db 00 40 43 c5 0e 56 04 18 93 00 40 46 c5 d2 f1 a9 fb e7 00 40 49 c6 97 8d 4f df 3b 00 40 4c c2 05 76 19 f0 fb 00 40 4e 4d 15 29 a4 85 cd 00 40 50 81 8d fe a2 79 83 00 40 52 01 f0 4c 75 6b 2d 00 40 53 3a 40 2b b0 cf 87 00 40 54 62 8b f2 58 bf 25 00 40 55 e2 ee 40 2b b0 cf 00 40 56 bb 25 8b f2 58 bf 00 40 57 1b 3e 1f 67 15 29 00 40 57 ee 1e b8 51 eb 85 00 40 58 73 96 2f c9 62 fc 00 40 58 f1 0b 9a f7 20 15 00 40 59 7e 85 1e b8 51 eb 00 40 5a 59 67 c3 ec e2 a5 00 40 5b 51 a7 40 da 74 0d 00 40 5c 5c 96 2f c9 62 fc 00 40 5d bf 9b a5 e3 53 f7 00 40 5e 47 be 76 c8 b4 39 00 40 5e 62 6f f5 13 cc 1d 00 40 5e 6a 72 01 5d 86 7c 00 40 5e 7a 76 19 f0 fb 38 00 40 5e bf dd 2f 1a 9f be 00 40 5f 2f f9 db 22 d0 e5 00 40 5f 9d 6b 2d bd 19 42 00 40 5f e0 26 e9 78 d4 fd 00 40 60 04 18 93 74 bc 6a 00 40 60 0a c4 f3 07 82 63 00 40 60 36 d0 36 9d 03 69 00 40 60 c8 4a c0 83 12 6e 00 40 61 0f 07 82 63 ab 59 00 40 61 68 73 b6 45 a1 ca 00 40 62 28 a4 dd 2f 1a 9f 00 40 62 97 6b dc 80 57 61 00 40 63 2b 91 bf d4 4f 30 00 40 63 eb c2 e6 bd c8 05 00 40 64 59 34 39 58 10 62 00 40 65 19 65 60 41 89 37 00 40 65 8d 83 12 6e 97 8d 00 40 65 ae e0 f0 4c 75 6b 00 40 65 d2 ea 27 98 3c 13 00 40 66 1c 52 42 e6 bd c8 00 40 66 bb 25 8b f2 58 bf 00 00 09 00 00 09

有兴趣的同学可以分析，看到这么复杂的脚本“数据区”是不是晕蛋了呢，如果没有的话恭喜你厉害，本菜可是分析了将近10分钟才得出这些结论的（反正吹牛又不用缴税，擦劳资的两个不眠夜啊）。接下来毁三观：
必须用的数据只在tag头中，必须用的数据只在tag头中，必须用的数据只在tag头中....
所以如果晕菜的话没关系，在实际应用中（比如视频合并）本菜的亲身经历告诉你其实这么复杂的脚本数据其实99%都没有卵用。
解析的时候发现有两个数据项需要格外注意，而且至关重要的并不在脚本tag数据区中，让我们在复习一下tag数据= tag头+tag数据区

第一个需要注意的数据项为tag 头中的 时间戳 和 时间戳扩展 必须解析对，否则合并后的视频在播放完第一个之后会出问题，原因为在播放时播放器只根据这个值来进行音频和视频的同步
第二个需要注意的是tag数据区中的duration 这个给出了该段视频的长度，如果不解析的话快进会出问题，但是依然可以播放。

2.1.2.2 音频数据和视频
音频和视频数据跟脚本数据比起来就是个渣渣，当然我只是说很少的东西需要我们分析，而是把数据直接扔给播放器就行。
数据 公式 = 类型信息（1个字节）+ 音频/视频数据
需要注意的是上面提到的时间戳的解析,比如视频合并时第二个视频的时间戳需要在第一个视频的最后一个时间戳的的基础上进行累加
2.1.2.2.1 音频数据
名称
⻓度
介绍
SoundFormat(音频格式）
UB[4] （4个bit)
0 = Linear PCM, platform endian
1 = ADPCM
2 = MP3
3 = Linear PCM, little endian
4 = Nellymoser 16-kHz mono
5 = Nellymoser 8-kHz mono
6 = Nellymoser
7 = G.711 A-law logarithmic PCM
8 = G.711 mu-law logarithmicPCM
9 = reserved
10 = AAC
11 = Speex
14 = MP3 8-Khz
15 = Device-specific sound
SoundRate (音频波特率）

UB[2]
0 = 5.5-kHz
1 = 11-kHz
2 = 22-kHz
3 = 44-kHz
SoundSize（采样长度）
UB[1]
0 = snd8Bit
1 = snd16Bit
SoundType（音频类型）
UB[1]
0 = sndMono
1 = sndStereo
ACC来说总是1
音频数据


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

2.1.2.2.1 视频数据
类型信息如下表中的第一个字节
名称
⻓度
介绍
FrameType(帧类型）
UB[4] （4个bit)
0 = Linear PCM, platform endian
1 = ADPCM
2 = MP3
3 = Linear PCM, little endian
4 = Nellymoser 16-kHz mono
5 = Nellymoser 8-kHz mono
6 = Nellymoser
7 = G.711 A-law logarithmic PCM
8 = G.711 mu-law logarithmicPCM
9 = reserved
10 = AAC
11 = Speex
14 = MP3 8-Khz
15 = Device-specific sound1:keyframe (for AVC, a seekable
frame)
2: inter frame (for AVC, a non-
seekable frame)
3: disposable inter frame(H.263
only)
4: generated keyframe (reservedfor
server use only)
5: video info/command frame
CodecID (编码类型）

UB[4]
1: JPEG (currently unused)
2: Sorenson H.263
3: Screen video
4: On2 VP6
5: On2 VP6 with alpha channel
6: Screen video version 2
7: AVC
VideoData（视频数据）
If CodecID == 2
or UI8
H263VIDEOPACKET
If CodecID == 3
SCREENVIDEOPACKET
If CodecID == 4
VP6FLVVIDEOPACKET
If CodecID == 5
VP6FLVALPHAVIDEOPACKET
If CodecID == 6
SCREENV2VIDEOPACKET
if CodecID == 7
AVCVIDEOPACKET


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

下图是本菜的最终成果：

总结：如果你有耐心看到这里说明，你真的很想知道到底如何解析一个FLV文件，这里传授我的秘籍：其实就是很苦逼的对照着管方的用户手册，一点点的分析，当你知道协议是怎么回事了之后，写程序就不难了，不是吗?


