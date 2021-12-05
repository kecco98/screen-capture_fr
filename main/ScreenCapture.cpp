//
// Created by kecco on 30/10/21.
//

#include <thread>
#include "ScreenCapture.h"

using namespace std;

ScreenCapture::ScreenCapture() : running(false), pause(false){

    avdevice_register_all();

}

ScreenCapture::~ScreenCapture(){

    if (av_write_trailer(outAVFormatContext) < 0) {
        cerr << "Error in writing av trailer" << endl;
        exit(-1);
    }

    avformat_close_input(&pAVFormatContext);
    if( !pAVFormatContext )
    {
        cout<<"\nfile closed sucessfully";
    }
    else
    {
        cout<<"\nunable to close the file";
        exit(1);
    }

   /* avformat_free_context(pAVFormatContext);
    avformat_free_context(pAudioFormatContext);
    avformat_free_context(outAVFormatContext);
    if( !pAudioFormatContext && !pAVFormatContext && !outAudioCodecContext)
    {
        cout<<"\navformat free successfully";
    }
    else
    {
        cout<<"\nunable to free avformat context";
        exit(1);
    }*/

    avformat_close_input(&pAudioFormatContext);
    if(pAVFormatContext == nullptr){
        cout<<"Flie close succesfully"<<endl;
    } else {
        cerr<<"Error: unable to close the file";
        exit(-1);
    }
    //pAudioCodecContext
   // avcodec_close(pAudioCodecContext);
    //av_free(pAudioCodecContext);
    avcodec_free_context(&pAudioCodecContext);
    if(pAudioCodecContext == nullptr){
        cout<<"Flie close succesfully"<<endl;
    } else {
        cerr<<"Error: unable to close the file";
        exit(-1);
    }
   // avcodec_free_context(&pAVCodecContext); gia fatta

    avformat_close_input(&outAVFormatContext);
    avcodec_free_context(&outAudioCodecContext);
    //avcodec_free_context(&outAVCodecContext);gia fatta
}

int ScreenCapture::setup(const char* output_file, int width, int height, const char* conc)
{
    pAVFormatContext = NULL;
    options = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");

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


    if(av_dict_set( &options,"video_size",conc,0 ) < 0)
    {
        cout<<"\nerror in setting video size";
        exit(3);
    }

    av_dict_set( &options, "preset", "superfast", 0 );



    if (av_dict_set(&options, "probesize", "60M", 0) < 0) {
        cerr << "Error in setting probesize value" << endl;
        exit(-1);
    }
    /*if(av_dict_set_int( &options,"height",1000,0 ) < 0)
      {
          cout<<"\nerror in setting dictionary value";
          exit(4);
      }*/

    if(avformat_open_input(&pAVFormatContext, ":0.0", pAVInputFormat, &options) != 0) { //start= 0.0+x,y punto partenza display
        cout<<"Error in opening the input device!";
        exit(1);
    }

    if(avformat_find_stream_info(pAVFormatContext,NULL) < 0) //da fare forse per il pausa e riprendi
    {
        cout<<"\nunable to find the stream information";
        exit(1);
    }

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

    /*-- FINE OPEN VIDEO DEVICE --*/

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

    /*-- INIZIA OPEN AUDIO DEVICE--*/

    audioOptions=nullptr;

    pAudioFormatContext = nullptr;

    pAudioFormatContext = avformat_alloc_context();
    if(pAudioFormatContext==nullptr){
        cerr << "Error: paudiocontext" << endl;
    }
    if ( av_dict_set(&audioOptions, "sample_rate", "44100", 0) < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }

    if (av_dict_set(&audioOptions, "async", "1", 0) < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }
    //av_dict_set(&audioOptions, "audio_device_number", "0", 0);
    //av_dict_set(&audioOptions, "thread_queue_size", "4096", 0);

//#if defined linux
    pAudioInputFormat = av_find_input_format("alsa");
    /*string deviceName;
    if(deviceName == "") deviceName = "default"; */
   // pAudioInputFormat =av_find_input_format("pulseaudio");


    if (avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions) != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }
//#endif


/*#if defined _WIN32
    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    if (value != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }
#endif*/


    if (avformat_find_stream_info(pAudioFormatContext, nullptr) != 0) {
        cerr << "Error: cannot find the audio stream information" << endl;
        exit(-1);
    }

    for (int i = 0; i < pAudioFormatContext->nb_streams; i++) {
        if (pAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        cerr << "Error: unable to find audio stream index" << endl;
        exit(-2);
    }



    video_st = avformat_new_stream(outAVFormatContext ,NULL);
    if( !video_st )
    {
        cout<<"\nerror in creating a av format new stream";
        exit(1);
    }

    outAVCodec=nullptr;

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);//outAvCodec nullo cosi in auto sceflie il settaggio migliore per il ctx
    if( !outAVCodecContext )
    {
        cout<<"\nerror in allocating the codec contexts";
        exit(1);
    }

    /* set property of the video file */
    /*outAVCodecContext = video_st->codec;
    //avcodec_parameters_to_context(outAVCodecContext, video_st->codecpar);

    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 800000; // 2500000
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;*/

    outAVCodecContext = video_st->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 10000000;
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 10;
    outAVCodecContext->global_quality = 500;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;
    outAVCodecContext->bit_rate_tolerance = 400000;

    if (outAVCodecContext->codec_id == AV_CODEC_ID_H264) { //copiato da vedere a cosa serve
        av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
    }


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
    avcodec_parameters_from_context(outAVFormatContext->streams[VideoStreamIndx]->codecpar, outAVCodecContext);

    AVCodecParameters* params = pAudioFormatContext->streams[audioStreamIndx]->codecpar;



    pAudioCodec = avcodec_find_decoder(params->codec_id);
    if (pAudioCodec == nullptr) {
        cerr << "Error: cannot find the audio decoder" << endl;
        exit(-1);
    }

    pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
    if (avcodec_parameters_to_context(pAudioCodecContext, params) < 0) {
        cout << "Cannot create codec context for audio input" << endl;
    }

    if (avcodec_open2(pAudioCodecContext, pAudioCodec, nullptr) < 0) {
        cerr << "Error: cannot open the input audio codec" << endl;
        exit(-1);
    }

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        cerr << "Error: cannot create audio stream" << endl;
        exit(1);
    }

    outAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (outAudioCodec == nullptr) {
        cerr << "Error: cannot find requested encoder" << endl;
        exit(1);
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        cerr << "Error: cannot create related VideoCodecContext" << endl;
        exit(1);
    }

    if (outAudioCodec->supported_samplerates) {
        outAudioCodecContext->sample_rate = outAudioCodec->supported_samplerates[0];
        for (i = 0; outAudioCodec->supported_samplerates[i]; i++) {
            if (outAudioCodec->supported_samplerates[i] == pAudioCodecContext->sample_rate)
                outAudioCodecContext->sample_rate = pAudioCodecContext->sample_rate;
        }
    }
    outAudioCodecContext->codec_id = AV_CODEC_ID_AAC;
    outAudioCodecContext->sample_fmt = outAudioCodec->sample_fmts ? outAudioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->channels = pAudioCodecContext->channels;
    outAudioCodecContext->channel_layout = av_get_default_channel_layout(outAudioCodecContext->channels);
    outAudioCodecContext->bit_rate = 96000;
    outAudioCodecContext->time_base = { 1, pAudioCodecContext->sample_rate };

    outAudioCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr) < 0) {
        cerr << "error in opening the avcodec" << endl;
        exit(1);
    }

    //find a free stream index
    for (i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outAudioStreamIndex = i;
        }
    }
    if (outAudioStreamIndex < 0) {
        cerr << "Error: cannot find a free stream for audio on the output" << endl;
        exit(1);
    }

   avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);

    //da qui unificare
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


    pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pAVPacket);


    pAVFrame = av_frame_alloc();
    //pAVFrame->width=width;
    // pAVFrame->height=height;
    // pAVFrame->format=AV_PIX_FMT_YUV420P;

    if( !pAVFrame )
    {
        cout<<"\nunable to release the avframe resources";
        exit(1);
    }

    outFrame = av_frame_alloc();//Allocate an AVFrame and set its fields to default values.
    outFrame->width=width;
    outFrame->height=height;
    outFrame->format=AV_CODEC_ID_MPEG4;

    if( !outFrame )
    {
        cout<<"\nunable to release the avframe resources for outframe";
        exit(1);
    }
    avformat_close_input(&pAudioFormatContext);
    return 10;
}

int ScreenCapture::genMenu() {
    char s;
    char p;
    char r;

    cout<<"Write s if you want to start!"<<endl;
    cin>>s;
    running=true;
    cv_s.notify_all();
    cout<<"Recording is runnig!"<<endl;
    cout<<"- write p to pause the recording"<<endl;
    cout<<"- write t to terminate the recording"<<endl;
    while(running){
        cin>>p;
        if(p=='p'){
            //pausa
            pause=true;
            cout<<"Recording in pause!"<<endl;
            cout<<"- write r to restart the recording"<<endl;
            while(pause){
                cin>>r;
                if(r=='r'){
                    pause=false;
                    cv_v.notify_all();
                    cv_a.notify_all();
                    cout<<"Restart recording!"<<endl;

                }
            }
        } else if(p=='t'){
            running=false;
            cout<<"Recording terminated"<<endl;
        }
    }

    return 0;
}

int ScreenCapture::start() {
    unique_lock<mutex> lr(lock_running);


    menu= new std::thread(&ScreenCapture::genMenu,this);

    cv_s.wait(lr,[this](){return running;});

    videoStream = new std::thread(&ScreenCapture::startVideoRecording,this);
    audioStream = new std::thread(&ScreenCapture::startAudioRecording,this);

    videoStream->join();
    audioStream->join();

    menu->join();
  return 1;

}


int ScreenCapture::startVideoRecording() {
    //https://stackoverflow.com/questions/54338342/ffmpeg-rgb-to-yuv420p-warning-data-is-not-aligned-this-can-lead-to-a-speedlo
    unique_lock<mutex> lp(lock_pause);
    int frameFinished;//  when you decode a single packet, you still don't have information enough to have a frame [depending on the type of codec, some of them //you do], when you decode a GROUP of packets that represents a frame, then you have a picture! that's why frameFinished will let //you know you decoded enough to have a frame.
    pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pAVPacket);


    pAVFrame = av_frame_alloc();
    //pAVFrame->width=width;
    // pAVFrame->height=height;
    // pAVFrame->format=AV_PIX_FMT_YUV420P;

    if( !pAVFrame )
    {
        cout<<"\nunable to release the avframe resources";
        exit(1);
    }

    outFrame = av_frame_alloc();//Allocate an AVFrame and set its fields to default values.
    outFrame->width=width;
    outFrame->height=height;
    outFrame->format=AV_CODEC_ID_MPEG4;

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



    int no_frames = 200;
   /* cout<<"\nenter No. of frames to capture : ";
    cin>>no_frames;
*/
    AVPacket outPacket;
    int ii=0;
    int ret;
    while( av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {
       // if( ii++ == no_frames )break;
        if(running== false) break;
        cv_v.wait(lp,[this](){return !pause;});
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
                    //cout<<"si";
                    if(avcodec_receive_packet(outAVCodecContext,&outPacket)>=0){
                        if(outPacket.pts != AV_NOPTS_VALUE)
                            outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                        if(outPacket.dts != AV_NOPTS_VALUE)
                            outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);
                        lock_sf.lock();
                        if(av_write_frame(outAVFormatContext , &outPacket) != 0) //avcodec_receive_packet()
                        {
                            cout<<"\nerror in writing video frame";

                        }
                        lock_sf.unlock();
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



    /*
    auto demux = new thread(&ScreenCapture::captureScreen, this, no_frames);

    auto rescale = new thread(&ScreenCapture::scaleVideo, this, no_frames);

    demux = new thread(&ScreenCapture::encodeVideo, this, no_frames);
    */
//THIS WAS ADDED LATER
    av_free(video_outbuf);  //lasciami qui

}

int ScreenCapture::startAudioRecording() {
    unique_lock<mutex> lp(lock_pause_audio);
    int ret;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;
    uint8_t** resampledData;

   // avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);
    init_fifo();

    //allocate space for a packet
    inPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!inPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }
    av_init_packet(inPacket);

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    outPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    //init the resampler
    SwrContext* resampleContext = nullptr;
    resampleContext = swr_alloc_set_opts(resampleContext,
                                         av_get_default_channel_layout(outAudioCodecContext->channels),
                                         outAudioCodecContext->sample_fmt,
                                         outAudioCodecContext->sample_rate,
                                         av_get_default_channel_layout(pAudioCodecContext->channels),
                                         pAudioCodecContext->sample_fmt,
                                         pAudioCodecContext->sample_rate,
                                         0,
                                         nullptr);
    if (!resampleContext) {
        cerr << "Cannot allocate the resample context" << endl;
        exit(1);
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
    }
    int ii=0;

    avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions);

    while ( running /*av_read_frame(pAudioFormatContext, inPacket) >= 0 *//*&& inPacket->stream_index == audioStreamIndx*/) {

        //if( ii++ == 2400  )break;
        // if (running == false) break;


        if (pause) {
           // avcodec_flush_buffers(m_codec_ctx.get());
            avformat_close_input(&pAudioFormatContext);
        cv_a.wait(lp, [this]() { return !pause; });
       avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions);

    }
        if( av_read_frame(pAudioFormatContext, inPacket) <0) break;

         // }
//decode audio routing
        av_packet_rescale_ts(outPacket, pAudioFormatContext->streams[audioStreamIndx]->time_base, pAudioCodecContext->time_base);

        if ((ret = avcodec_send_packet(pAudioCodecContext, inPacket)) < 0) {
            cout << "Cannot decode current audio packet " << ret << endl;
            continue;
        }

        while (ret >= 0) {
            ret = avcodec_receive_frame(pAudioCodecContext, rawFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0) {
                cerr << "Error during decoding" << endl;
                exit(1);
            }

            if (outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
            }
            initConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

            swr_convert(resampleContext,
                        resampledData, rawFrame->nb_samples,
                        (const uint8_t**)rawFrame->extended_data, rawFrame->nb_samples);
            add_samples_to_fifo(resampledData, rawFrame->nb_samples);

            //raw frame ready
            av_init_packet(outPacket);
            outPacket->data = nullptr;
            outPacket->size = 0;

            const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

            scaledFrame = av_frame_alloc();
            if (!scaledFrame) {
                cerr << "Cannot allocate an AVPacket for encoded video" << endl;
                exit(1);
            }

            scaledFrame->nb_samples = outAudioCodecContext->frame_size;
            scaledFrame->channel_layout = outAudioCodecContext->channel_layout;
            scaledFrame->format = outAudioCodecContext->sample_fmt;
            scaledFrame->sample_rate = outAudioCodecContext->sample_rate;
            av_frame_get_buffer(scaledFrame, 0);

            while (av_audio_fifo_size(fifo) >= outAudioCodecContext->frame_size) {

                ret = av_audio_fifo_read(fifo, (void**)(scaledFrame->data), outAudioCodecContext->frame_size);

                scaledFrame->pts = pts;
                pts += scaledFrame->nb_samples;
                if (avcodec_send_frame(outAudioCodecContext, scaledFrame) < 0) {
                    cout << "Cannot encode current audio packet " << endl;
                    exit(1);
                }
                while (ret >= 0) {
                    ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0) {
                        cerr << "Error during encoding" << endl;
                        exit(1);
                    }
                    av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base, outAVFormatContext->streams[outAudioStreamIndex]->time_base);

                    outPacket->stream_index = outAudioStreamIndex;

                    lock_sf.lock();

                    if (av_write_frame(outAVFormatContext, outPacket) != 0)
                    {
                        cerr << "Error in writing audio frame" << endl;
                    }
                    lock_sf.unlock();
                    av_packet_unref(outPacket);
                }
                ret = 0;
            }
            av_frame_free(&scaledFrame);
            av_packet_unref(outPacket);

        }
    }
    swr_free(&resampleContext);
}
int ScreenCapture::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
                                     outAudioCodecContext->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int ScreenCapture::add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) {
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */

    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void**)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int ScreenCapture::initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size) {
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,
                                                       sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if (av_samples_alloc(*converted_input_samples, nullptr,
                         output_codec_context->channels,
                         frame_size,
                         output_codec_context->sample_fmt, 0) < 0) {

        exit(1);
    }
    return 0;
}

int ScreenCapture::openInputVideo() {
    pAVFormatContext = NULL;
    options = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");

    if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(2);
    }

    if(av_dict_set( &options,"video_size",conc,0 ) < 0)
    {
        cout<<"\nerror in setting video size";
        exit(3);
    }

    av_dict_set( &options, "preset", "superfast", 0 );

    if (av_dict_set(&options, "probesize", "60M", 0) < 0) {
        cerr << "Error in setting probesize value" << endl;
        exit(-1);
    }

    if(avformat_open_input(&pAVFormatContext, ":0.0", pAVInputFormat, &options) != 0) { //start= 0.0+x,y punto partenza display
        cout<<"Error in opening the input device!";
        exit(1);
    }

    if(avformat_find_stream_info(pAVFormatContext,NULL) < 0) //da fare forse per il pausa e riprendi
    {
        cout<<"\nunable to find the stream information";
        exit(1);
    }

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
    return 0;
}

int ScreenCapture::openInputAudio() {
    audioOptions=nullptr;

    pAudioFormatContext = nullptr;

    pAudioFormatContext = avformat_alloc_context();
    if(pAudioFormatContext==nullptr){
        cerr << "Error: paudiocontext" << endl;
    }
    if ( av_dict_set(&audioOptions, "sample_rate", "44100", 0) < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }

    if (av_dict_set(&audioOptions, "async", "1", 0) < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }
    //av_dict_set(&audioOptions, "audio_device_number", "0", 0);
    //av_dict_set(&audioOptions, "thread_queue_size", "4096", 0);

//#if defined linux
    pAudioInputFormat = av_find_input_format("alsa");
    /*string deviceName;
    if(deviceName == "") deviceName = "default"; */
    // pAudioInputFormat =av_find_input_format("pulseaudio");


    if (avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions) != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }
//#endif


/*#if defined _WIN32
    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    if (value != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }
#endif*/


    if (avformat_find_stream_info(pAudioFormatContext, nullptr) != 0) {
        cerr << "Error: cannot find the audio stream information" << endl;
        exit(-1);
    }

    for (int i = 0; i < pAudioFormatContext->nb_streams; i++) {
        if (pAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        cerr << "Error: unable to find audio stream index" << endl;
        exit(-2);
    }

    AVCodecParameters* params = pAudioFormatContext->streams[audioStreamIndx]->codecpar;

    pAudioCodec = avcodec_find_decoder(params->codec_id);
    if (pAudioCodec == nullptr) {
        cerr << "Error: cannot find the audio decoder" << endl;
        exit(-1);
    }

    pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
    if (avcodec_parameters_to_context(pAudioCodecContext, params) < 0) {
        cout << "Cannot create codec context for audio input" << endl;
    }

    if (avcodec_open2(pAudioCodecContext, pAudioCodec, nullptr) < 0) {
        cerr << "Error: cannot open the input audio codec" << endl;
        exit(-1);
    }

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        cerr << "Error: cannot create audio stream" << endl;
        exit(1);
    }
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

    outAVCodec=nullptr;

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);//outAvCodec nullo cosi in auto sceflie il settaggio migliore per il ctx
    if( !outAVCodecContext )
    {
        cout<<"\nerror in allocating the codec contexts";
        exit(1);
    }

    /* set property of the video file */
    /*outAVCodecContext = video_st->codec;
    //avcodec_parameters_to_context(outAVCodecContext, video_st->codecpar);

    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 800000; // 2500000
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;*/

    outAVCodecContext = video_st->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 10000000;
    outAVCodecContext->width = width;
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 10;
    outAVCodecContext->global_quality = 500;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30;
    outAVCodecContext->bit_rate_tolerance = 400000;

    if (outAVCodecContext->codec_id == AV_CODEC_ID_H264) { //copiato da vedere a cosa serve
        av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
    }


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
    avcodec_parameters_from_context(outAVFormatContext->streams[VideoStreamIndx]->codecpar, outAVCodecContext);
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

    return 0;
}

int ScreenCapture::openInput(int width, int height) {

    string in2, co, x, y;

    cout<<"Insert the width of the window you want to record!"<<endl;
    cin >> width;
    cout<<"Insert the height of the window you want to record!"<<endl;
    cin >> height;
    co = to_string(width) + "x" + to_string(height);
    conc = co.c_str();
    cout<< conc;

    openInputVideo();
    openInputAudio();
    return 0;
}

/*
void ScreenCapture::captureScreen(int no_frames )
{
    int ii = 0;
    int ret;

    while (av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {
        if( ii++ == no_frames )break;
        if(pAVPacket->stream_index == VideoStreamIndx) {
            ret = avcodec_send_packet(pAVCodecContext, pAVPacket);
            if (ret >= 0) {
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) //gestire errori

                    exit(-2);
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit( -1);
                }
            }
        }
    }
}

void ScreenCapture::scaleVideo(int no_frames) {
//int video_outbuf_size;

    AVPacket outPacket;

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
    while (av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {
        sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize,0, pAVCodecContext->height, outFrame->data,outFrame->linesize);
        av_init_packet(&outPacket);
        outPacket.data = NULL;    // packet data will be allocated by the encoder
        outPacket.size = 0;
        avcodec_send_frame(outAVCodecContext,outFrame);
    }



}
void ScreenCapture::encodeVideo(int no_frames)
{

    AVPacket outPacket;
    int ii = 0;

    while (ii++ == no_frames ) {
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

    if( av_write_trailer(outAVFormatContext) < 0)
    {
        cout<<"\nerror in writing av trailer";
        exit(1);
    }
 //


}*/

/*void ScreenCapture::captureScreen(int no_frames )
{
    int ii = 0;
    int ret;

    while (av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {
        if( ii++ == no_frames )break;
        if(pAVPacket->stream_index == VideoStreamIndx) {
            ret = avcodec_send_packet(pAVCodecContext, pAVPacket);
            if (ret >= 0) {
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) //gestire errori

                    exit(-2);
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit( -1);
                }
            }
        }
    }




    }*/