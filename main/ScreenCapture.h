
#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>

#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
//#include "../include/avfiltergraph.h"
//#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

// libav resample

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"

// lib swresample

#include "libswscale/swscale.h"

#include <X11/Xlib.h>

}

class ScreenCapture
{
private:
    AVFormatContext *pAVFormatContext;
    AVInputFormat *pAVInputFormat;
    AVDictionary *options;
    AVCodecContext *pAVCodecContext;
    AVCodec *pAVCodec;
    AVFormatContext *outAVFormatContext;
    AVOutputFormat *output_format;
    AVStream *video_st;
    AVCodecContext *outAVCodecContext;
    AVCodec *outAVCodec;
    AVFrame *pAVFrame;
    AVFrame *outFrame;
    AVPacket *pAVPacket;


    int VideoStreamIndx;
    int codec_id;

public:

    ScreenCapture();
    ~ScreenCapture();
    int setup(const char* start, const char* output, int width, int height);
    int startRecording();


};

