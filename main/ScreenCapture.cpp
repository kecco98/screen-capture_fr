//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"

using namespace std;

ScreenCapture::ScreenCapture(){

    avdevice_register_all();

}

ScreenCapture::~ScreenCapture(){



}


int ScreenCapture::setup(const char* start, const char* output_file)
{
    pAVFormatContext = NULL;
    options = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");
    cout<<start;

    if(avformat_open_input(&pAVFormatContext, start, pAVInputFormat, NULL) != 0) {
        cout<<"Error in opening the input device!";
        exit(1);
    }

  if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(2);
    }


/*    if(avformat_find_stream_info(pAVFormatContext,NULL) < 0) da fare forse per il pausa e riprendi
    {
        cout<<"\nunable to find the stream information";
        exit(1);
    }*/
    VideoStreamIndx = -1;

    /* find the first video stream index . Also there is an API available to do the below operations */
    for(int i = 0; i < pAVFormatContext->nb_streams; i++ ) // find video stream posistion/index.
    {
        if( pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            VideoStreamIndx = i;
            break;
        }

    }

    if( VideoStreamIndx == -1)
    {
        cout<<"\nunable to find the video stream index. (-1)";
        exit(1);
    }
    // Saving video codec
    pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;


    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);//trova il codec
    if( pAVCodec == NULL )
    {
        cout<<"\nunable to find the decoder";
        exit(1);
    }

    //Initialize the AVCodecContext to use the given AVCodec. ,copia info codec in ctx
    if( avcodec_open2(pAVCodecContext , pAVCodec , NULL) < 0 )
    {
        cout<<"\nUnable to open the av codec"<<endl;
        exit(1);
    }

    //INIT

    avformat_alloc_output_context2(&outAVFormatContext, NULL, NULL, output_file);
    if (!outAVFormatContext)
    {
        cout<<"\nError in allocating av format output context"<<endl;
        exit(1);
    }

    output_format = av_guess_format(NULL, output_file ,NULL);
    if( !output_format )
    {
        cout<<"\nerror in guessing the video format. try with correct format";
        exit(1);
    }

    video_st = avformat_new_stream(outAVFormatContext ,NULL);
    if( !video_st )
    {
        cout<<"\nerror in creating a av format new stream";
        exit(1);
    }

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);//outAvCodec nullo cosi in auto sceflie il settaggio migliore per il ctx
    if( !outAVCodecContext )
    {
        cout<<"\nerror in allocating the codec contexts";
        exit(1);
    }


    /* set property of the video file */
    outAVCodecContext = video_st->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 400000; // 2500000
    outAVCodecContext->width = 1920;
    outAVCodecContext->height = 1080;
    outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;

    char *wid; Display *dpy; Window w;
    int width, height, snum;
    dpy = XOpenDisplay(0);

    snum = DefaultScreen(dpy);
    width = DisplayWidth(dpy, snum);
    height = DisplayHeight(dpy, snum);
    printf("display size is %d x %d\n", width, height);

}
