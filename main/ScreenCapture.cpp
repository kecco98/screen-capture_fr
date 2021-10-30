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


int ScreenCapture::setup(const char* start)
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


    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if( pAVCodec == NULL )
    {
        cout<<"\nunable to find the decoder";
        exit(1);
    }

    //Initialize the AVCodecContext to use the given AVCodec.
    if( avcodec_open2(pAVCodecContext , pAVCodec , NULL) < 0 )
    {
        cout<<"\nunable to open the av codec";
        exit(1);
    }

}
