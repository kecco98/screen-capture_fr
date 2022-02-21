#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <mutex>
#include <condition_variable>
#include<thread>
#include <queue>

#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
//#include "../include/avfiltergraph.h"
//#include "libavfilter/avfiltergraph.h"ti
#include "libavutil/samplefmt.h"
#include "libswresample/version.h"
#include "libswresample/swresample.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/audio_fifo.h"

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

#ifdef _WIN32

#elif defined linux

#include <X11/Xlib.h>

#endif

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

    AVDictionary *audioOptions;
    AVFormatContext *pAudioFormatContext;
    AVInputFormat *pAudioInputFormat;
    AVCodec *pAudioCodec;
    AVCodecContext *pAudioCodecContext;
    AVCodec *outAudioCodec;
    AVCodecContext *outAudioCodecContext;
    AVAudioFifo * fifo;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;

    int VideoStreamIndx=-1;
    int audioStreamIndx=-1;
    int outAudioStreamIndex = -1;
    int codec_id;
    int width, height;
    bool audio;
    const char* output;
    const char* conc;
    const char* sta;
    const char* linSta;
    const char* winSta;
    std::string x, y;
    int64_t pts = 0;

//threads
    std::thread *videoStream;
    std::thread *audioStream;
    std::thread *menu;
//mutex
    std::mutex lock_sf;
    std::mutex lock_init;
    std::mutex lock_running;
    std::mutex lock_pause;
    std::mutex lock_pause_audio;
//variabili di controllo esecuzione
    bool running;
    bool pause;
    bool initVideo;
//condition variables
    std::condition_variable cv_s;
    std::condition_variable cv_v;
    std::condition_variable cv_a;
    std::condition_variable cv_t;


public:

    ScreenCapture();
    ~ScreenCapture();
    int start();
    int startAudioRecording();
    int startVideoRecording();
    int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size);
    int init_fifo();
    int add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);
    int genMenu();
    int openInputVideo();
    int openInputAudio();
    int openInput(int widthi, int heighti,const char* outputi,bool audioi, std::string xi, std::string yi);
    int streamTrail();
    /*void captureScreen(int no_frames, uint8_t *video_outbuf);
    void scaleVideo(int no_frames , uint8_t *video_outbuf);
    void encodeVideo(int no_frames);*/


};

