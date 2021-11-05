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


int ScreenCapture::setup(const char* start, const char* output_file, int width, int height, const char* conc)
{
    pAVFormatContext = NULL;
    options = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");

    cout<<start;

//https://www.titanwolf.org/Network/q/eb46fd58-9cf7-4160-a1a2-a924d8c5639a/y
    //The distance from the left edge of the screen or desktop
    //av_dict_set(&options,"offset_x","20",0);
    //The distance from the top edge of the screen or desktop
    //av_dict_set(&options,"offset_y","40",0);

  if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(2);
    }


  if(av_dict_set( &options,"video_size",conc,0 ) < 0) //creare una concat di heigthxwitdh
    {
        cout<<"\nerror in setting video size";
        exit(3);
    }


    if(avformat_open_input(&pAVFormatContext, start, pAVInputFormat, &options) != 0) { //start= 0.0+x,y punto partenza display
        cout<<"Error in opening the input device!";
        exit(1);
    }
  /*if(av_dict_set_int( &options,"height",1000,0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(4);
    }*/


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
    //avcodec_parameters_to_context(outAVCodecContext, video_st->codecpar);

    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 400000; // 2500000
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;

  /* char *wid; Display *dpy; Window w;
    int width2, height2, snum;
    dpy = XOpenDisplay(0);

    snum = DefaultScreen(dpy);
    width2 = DisplayWidth(dpy, snum);
    height2 = DisplayHeight(dpy, snum);
    printf("display size is %d x %d\n", width2, height2);*/

   //Per un altra codifica!!
 /*   if (codec_id == AV_CODEC_ID_H264)
    {
        av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
    }*/

    outAVCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);//prenderlo da sopra!!!
    if( !outAVCodec )
    {
        cout<<"\nError in finding the av codecs. try again with correct codec"<<endl;
        exit(1);
    }

    /* Some container formats (like MP4) require global headers to be present
     Mark the encoder so that it behaves accordingly. */

    if ( outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    if( avcodec_open2(outAVCodecContext, outAVCodec, NULL) < 0) //inizializza out context
    {
        cout<<"\nerror in opening the avcodec";
        exit(1);
    }

    /* create empty video file */
    if ( !(outAVFormatContext->flags & AVFMT_NOFILE) )
    {
        cout<<output_file<<"  file ouy\n ";
        if( avio_open2(&outAVFormatContext->pb , output_file , AVIO_FLAG_WRITE ,NULL, NULL) < 0 )
        {
            cout<<"\nerror in creating the video file";
            exit(1);
        }
    }

    if(!outAVFormatContext->nb_streams)
    {
        cout<<"\noutput file dose not contain any stream";
        exit(1);
    }

    /* imp: mp4 container or some advanced container file required header information*/
    if(avformat_write_header(outAVFormatContext , &options) < 0)
    {
        cout<<"\nerror in writing the header context";
        exit(1);
    }

    cout<<"\n\nOutput file information :\n\n";
    av_dump_format(outAVFormatContext , 0 ,output_file ,1);

}

int ScreenCapture::startRecording() {


    int frameFinished;//when you decode a single packet, you still don't have information enough to have a frame [depending on the type of codec, some of them //you do], when you decode a GROUP of packets that represents a frame, then you have a picture! that's why frameFinished will let //you know you decoded enough to have a frame.




    pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pAVPacket);

    pAVFrame = av_frame_alloc();
    if( !pAVFrame )
    {
        cout<<"\nunable to release the avframe resources";
        exit(1);
    }

    outFrame = av_frame_alloc();//Allocate an AVFrame and set its fields to default values.
    if( !outFrame )
    {
        cout<<"\nunable to release the avframe resources for outframe";
        exit(1);
    }

    //int video_outbuf_size;
    int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt,outAVCodecContext->width,outAVCodecContext->height,32);
    uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes);
    if( video_outbuf == NULL )
    {
        cout<<"\nunable to allocate memory";
        exit(1);
    }

    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    if(av_image_fill_arrays( outFrame->data, outFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, outAVCodecContext->width,outAVCodecContext->height,1 )<0) // returns : the size in bytes required for src
    {
        cout<<"\nerror in filling image array";
    }

    SwsContext* swsCtx_ ;

    // Allocate and return swsContext.
    // a pointer to an allocated context, or NULL in case of error
    // Deprecated : Use sws_getCachedContext() instead.
    swsCtx_ = sws_getContext(pAVCodecContext->width,
                             pAVCodecContext->height,
                             pAVCodecContext->pix_fmt,
                             outAVCodecContext->width,
                             outAVCodecContext->height,
                             outAVCodecContext->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);


    int ii = 0;
    int no_frames = 100;
    cout<<"\nenter No. of frames to capture : ";
    cin>>no_frames;

    AVPacket outPacket;
    int j = 0;

    int got_picture;
    int ret;
    while( av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {
        if( ii++ == no_frames )break;
        if(pAVPacket->stream_index == VideoStreamIndx)
        {

           // if( avcodec_decode_video2( pAVCodecContext , pAVFrame , &frameFinished , pAVPacket ) < 0)
            //{
              //  cout<<"unable to decode video";
           // }
            ret=avcodec_send_packet(pAVCodecContext,pAVPacket);
            if(ret>=0){
                ret= avcodec_receive_frame(pAVCodecContext,pAVFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) //gestire errori

                               return -2;
                        else if (ret < 0) {
                                fprintf(stderr, "Error during decoding\n");
                                return -1 ;
                             }

                sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize,0, pAVCodecContext->height, outFrame->data,outFrame->linesize);
                av_init_packet(&outPacket);
                outPacket.data = NULL;    // packet data will be allocated by the encoder
                outPacket.size = 0;

               // avcodec_encode_video2(outAVCodecContext , &outPacket ,outFrame , &got_picture);//avcodec_send_frame()
              if(avcodec_send_frame(outAVCodecContext,outFrame)>=0){
                    cout<<"si";
                    if(avcodec_receive_packet(outAVCodecContext,&outPacket)>=0){
                        if(outPacket.pts != AV_NOPTS_VALUE)
                            outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                        if(outPacket.dts != AV_NOPTS_VALUE)
                            outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);
                        if(av_write_frame(outAVFormatContext , &outPacket) != 0) //avcodec_receive_packet()
                        {
                            cout<<"\nerror in writing video frame";

                        }
                        av_packet_unref(&outPacket);
                    }
                }

             /* if(got_picture)
                {
                    if(outPacket.pts != AV_NOPTS_VALUE)
                        outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                    if(outPacket.dts != AV_NOPTS_VALUE)
                        outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);

                    printf("Write frame %3d (size= %2d)\n", j++, outPacket.size/1000);
                    if(av_write_frame(outAVFormatContext , &outPacket) != 0) //avcodec_receive_packet()
                    {
                        cout<<"\nerror in writing video frame";
                    }

                    av_packet_unref(&outPacket);
                } // got_picture*/

              // av_packet_unref(&outPacket);
            } // frameFinished

        }
    }// End of while-loop


    if( av_write_trailer(outAVFormatContext) < 0)
    {
        cout<<"\nerror in writing av trailer";
        exit(1);
    }


//THIS WAS ADDED LATER
    av_free(video_outbuf);

}

