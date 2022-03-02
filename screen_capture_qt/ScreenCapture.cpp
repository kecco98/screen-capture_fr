#include <thread>
#include "ScreenCapture.h"
#include <future>
using namespace std;



ScreenCapture::ScreenCapture() : running(false), pause(false){

    avdevice_register_all();

}

ScreenCapture::~ScreenCapture(){

}

function<void(void)> ScreenCapture::make_error_handler(function<void(void)> f) {
    return [&]() {
        try {
            f();
            lock_guard<mutex> lg{error_queue_m};
            terminated_threads++;
            error_queue_cv.notify_one();
        } catch (const std::exception &e) {
            lock_guard<mutex> lg{error_queue_m};
            error_queue.emplace(e.what());
            error_queue_cv.notify_one();
        }
    };
}

int ScreenCapture::start() {
    unique_lock<mutex> lr(lock_running);
    gotFirstpacketvideo=false;




    running=true;


    captureVideo_thread = make_unique<thread>([this]()  {
        this->make_error_handler([this]() {
            this->startVideoRecording();
        })();
    });



    if(audio){

        captureAudio_thread = make_unique<thread>([this]() {
            this->make_error_handler([this]() {
                this->startAudioRecording();
            })();
        });

    }


    unique_lock<mutex> error_queue_ul{error_queue_m};
    error_queue_cv.wait(error_queue_ul, [&]() { return (!error_queue.empty() || terminated_threads == (audio ? 2 : 1)); });
    if (!error_queue.empty()) {
        this->terminate_recording();
        string error_message = error_queue.front();
        error_queue.pop();
        while (!error_queue.empty()) {
            error_message += string{"\n"} + error_queue.front();
            error_queue.pop();
        }
        throw runtime_error{error_message};
    }

  return 1;

}

void ScreenCapture::startVideoRecording() {

    unique_lock<mutex> lp(lock_pause);



    int frameFinished;//  when you decode a single packet, you still don't have information enough to have a frame [depending on the type of codec, some of them //you do], when you decode a GROUP of packets that represents a frame, then you have a picture! that's why frameFinished will let //you know you decoded enough to have a frame.
    pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pAVPacket);


    pAVFrame = av_frame_alloc();


    if( !pAVFrame )
    {
        throw logic_error{"Unable to release the avframe resources"};
    }

    outFrame = av_frame_alloc();//Allocate an AVFrame and set its fields to default values.
    outFrame->width=width;
    outFrame->height=height;
    outFrame->format=AV_CODEC_ID_MPEG4;

    if( !outFrame )
    {
        throw logic_error{"Unable to release the avframe resources for outframe"};
    }


    int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt,outAVCodecContext->width,outAVCodecContext->height,32);
    uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes);
    if( video_outbuf == nullptr )
    {
        throw logic_error{"Unable to allocate memory"};
    }


    if(av_image_fill_arrays( outFrame->data, outFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, outAVCodecContext->width,outAVCodecContext->height,1 )<0) // returns : the size in bytes required for src
    {
        throw runtime_error{"Error in filling image array"};
    }

    SwsContext* swsCtx_ ;


    swsCtx_ = sws_getContext(pAVCodecContext->width,
                             pAVCodecContext->height,
                             pAVCodecContext->pix_fmt,
                             outAVCodecContext->width,
                             outAVCodecContext->height,
                             outAVCodecContext->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);




    AVPacket outPacket;
    int ii=0;
    int ret;
    while( av_read_frame( pAVFormatContext , pAVPacket ) >= 0 )
    {

        if(!running) break;
        cv_v.wait(lp,[this](){return !pause;});
        if(pAVPacket->stream_index == VideoStreamIndx)
        {


            ret=avcodec_send_packet(pAVCodecContext,pAVPacket);
            if(ret>=0){
                ret= avcodec_receive_frame(pAVCodecContext,pAVFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) //gestire errori
                    break;
                else if (ret < 0) {
                    throw runtime_error("Error during decoding");
                }

                sws_scale(swsCtx_,pAVFrame->data, pAVFrame->linesize,0, pAVCodecContext->height, outFrame->data,outFrame->linesize);
                av_init_packet(&outPacket);
                outPacket.data = NULL;
                outPacket.size = 0;


                if(avcodec_send_frame(outAVCodecContext,outFrame)>=0){
                    if(avcodec_receive_packet(outAVCodecContext,&outPacket)>=0){
                        if(!gotFirstpacketvideo){

                            gotFirstpacketvideo=true;
                        }
                        if(outPacket.pts != AV_NOPTS_VALUE)
                            outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                        if(outPacket.dts != AV_NOPTS_VALUE)
                            outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);
                        lock_sf.lock();
                        if(av_write_frame(outAVFormatContext , &outPacket) != 0) //avcodec_receive_packet()
                        {
                            throw runtime_error("Error in writing video frame");
                        }
                        lock_sf.unlock();
                        av_packet_unref(&outPacket);
                    }
                }

            } // frameFinished

        }
    }


    av_free(video_outbuf);

}

int ScreenCapture::startAudioRecording() {
    unique_lock<mutex> lp(lock_pause_audio);

    int ret;
    uint8_t** resampledData;
    bool firstBuffer=true;


    init_fifo();

    inPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!inPacket) {
        throw logic_error("Cannot allocate an AVPacket for encoded video");
    }
    av_init_packet(inPacket);


    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        throw logic_error("Cannot allocate an AVPacket for encoded video");
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        throw logic_error("Cannot allocate an AVPacket for encoded video");
    }

    outPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        throw logic_error("Cannot allocate an AVPacket for encoded video");
    }


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
        throw logic_error("Cannot allocate the resample context");
    }
    if ((swr_init(resampleContext)) < 0) {
        swr_free(&resampleContext);
        throw runtime_error("Could not open resample context");

    }

    int ii=0;
    while ( running ) {



        if (pause) {

               avformat_close_input(&pAudioFormatContext);
            cv_a.wait(lp, [this]() { return !pause; });
            #if defined linux
                        avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions);
            #elif defined _WIN32

                        avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
            #else


                pAudioInputFormat = av_find_input_format("avfoundation");



                avformat_open_input(&pAudioFormatContext, ":0", pAudioInputFormat, &audioOptions);
            #endif
        }


        if( av_read_frame(pAudioFormatContext, inPacket) <0 || inPacket->stream_index != audioStreamIndx) {
        #if defined linux
            break;
        #else
            if (running) {
                continue;
            } else {
                break;
            }
        #endif
        }



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
                throw logic_error("Error during decoding");
            }

            if (outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
            }
            initConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

            swr_convert(resampleContext,resampledData, rawFrame->nb_samples,(const uint8_t**)rawFrame->extended_data, rawFrame->nb_samples);
            add_samples_to_fifo(resampledData, rawFrame->nb_samples);


            av_init_packet(outPacket);
            outPacket->data = nullptr;
            outPacket->size = 0;

            const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

            scaledFrame = av_frame_alloc();
            if (!scaledFrame) {
                throw logic_error("Cannot allocate an AVPacket for encoded video");
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
                    throw runtime_error("Cannot encode current audio packet");
                }
                while (ret >= 0) {
                    ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0) {
                        throw logic_error("Error during encoding");
                    }
                    av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base, outAVFormatContext->streams[outAudioStreamIndex]->time_base);

                    outPacket->stream_index = outAudioStreamIndex;

                    lock_sf.lock();
                    if (gotFirstpacketvideo) {
                        if (!firstBuffer) {
                            if (av_write_frame(outAVFormatContext, outPacket) != 0) {
                                throw runtime_error("Error in writing audio frame");
                            }
                        } else {
                            firstBuffer = false;
                        }
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

    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
                                     outAudioCodecContext->channels, 1))) {

        throw runtime_error("Could not allocate FIFO");
    }
    return 0;
}

int ScreenCapture::add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) {
    int error;


    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {

        throw runtime_error("Could not reallocate FIFO");
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void**)converted_input_samples, frame_size) < frame_size) {

        throw runtime_error("Could not write data to FIFO");
    }
    return 0;
}

int ScreenCapture::initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size) {
    int error;

    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,sizeof(**converted_input_samples)))) {

        throw runtime_error("Could not allocate converted input sample pointers");
    }

    if (av_samples_alloc(*converted_input_samples, nullptr,output_codec_context->channels,frame_size,output_codec_context->sample_fmt, 0) < 0) {
        throw runtime_error("could not allocate memory for samples in all channels (audio)");
       /* exit(1);*/
    }
    return 0;
}

int ScreenCapture::openInputVideo() {
    pAVFormatContext = NULL;
    options = NULL;


    pAVFormatContext = avformat_alloc_context();


    if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        throw runtime_error("Error in setting dictionary value");
    }

    if(av_dict_set( &options,"video_size",conc,0 ) < 0)
    {
        throw runtime_error("Error in setting video size");
    }

    av_dict_set( &options, "preset", "superfast", 0 );

    if (av_dict_set(&options, "probesize", "60M", 0) < 0) {
        throw runtime_error("Error in setting probesize value");
    }

#ifdef _WIN32
    pAVInputFormat = av_find_input_format("gdigrab");

    if (avformat_open_input(&pAVFormatContext, winSta, pAVInputFormat, &options) != 0) {
        cerr << "Couldn't open input stream" << endl;
        throw runtime_error("Couldn't open input stream");
    }

#elif defined linux

    pAVInputFormat = av_find_input_format("x11grab");

    if(avformat_open_input(&pAVFormatContext, linSta, pAVInputFormat, &options) != 0) { //start= 0.0+x,y punto partenza display
        throw runtime_error("Error in opening the input device!");
    }
#else

    if (av_dict_set(&options, "pixel_format", "0rgb", 0) < 0) {
        throw runtime_error("Error in setting pixel format");
    }

    if (av_dict_set(&options, "video_device_index", "1", 0) < 0) {
        throw runtime_error("Error in setting video device indext");
    }

    string video_format = "crop=" + to_string(width) + ":" + to_string(height) + ":" + x +":" + y;
    cout << video_format << endl;
    if (av_dict_set(&options, "vf", video_format.c_str(), 0) < 0) {
        throw runtime_error("Error in setting video device index");
    }




    pAVInputFormat = av_find_input_format("avfoundation");

    if (avformat_open_input(&pAVFormatContext, "", pAVInputFormat, &options) != 0) {
        throw runtime_error("Error in opening input device");
    }

#endif

    if(avformat_find_stream_info(pAVFormatContext,NULL) < 0)
    {
        throw runtime_error("Unable to find the stream information");
    }

    VideoStreamIndx = -1;


    for(int i = 0; i < pAVFormatContext->nb_streams; i++ )
    {
        if( pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            VideoStreamIndx = i;
            break;
        }

    }

    if( VideoStreamIndx == -1)
    {
        throw logic_error("Unable to find the video stream index. (-1)");
    }

    pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;

    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);//trova il codec
    if( pAVCodec == nullptr )
    {
        throw runtime_error("Unable to find the decoder");
    }

    if( avcodec_open2(pAVCodecContext , pAVCodec , NULL) < 0 )
    {
        throw runtime_error("Unable to open the av codec");
    }

    //INIT

    avformat_alloc_output_context2(&outAVFormatContext, NULL, NULL, output);
    if (!outAVFormatContext)
    {
        throw logic_error("Error in allocating av format output context");
    }

    output_format = av_guess_format(NULL, output ,NULL);
    if( !output_format )
    {
        throw runtime_error("Error in guessing the video format. try with correct format");
    }
    video_st = avformat_new_stream(outAVFormatContext ,NULL);
    if( !video_st )
    {
        throw runtime_error("Error in creating a av format new stream");
    }

    outAVCodec=nullptr;

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);
    if( !outAVCodecContext )
    {
        throw runtime_error("Error in allocating the codec contexts");
    }



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



    outAVCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);//prenderlo da sopra!!!
    if( !outAVCodec )
    {
        throw runtime_error("Error in finding the av codecs. try again with correct codec");
    }


    if ( outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    if( avcodec_open2(outAVCodecContext, outAVCodec, NULL) < 0) //inizializza out context
    {
        throw runtime_error("Error in opening the avcodec");
    }
    avcodec_parameters_from_context(outAVFormatContext->streams[VideoStreamIndx]->codecpar, outAVCodecContext);
    if ( !(outAVFormatContext->flags & AVFMT_NOFILE) )
    {

        if( avio_open2(&outAVFormatContext->pb , output , AVIO_FLAG_WRITE ,NULL, NULL) < 0 )
        {
            throw runtime_error("Error in creating the video file");
        }
    }

    if(!outAVFormatContext->nb_streams)
    {
        throw logic_error("Output file does not contain any stream");
    }


    return 0;
}

int ScreenCapture::openInputAudio() {


    audioOptions=nullptr;

    pAudioFormatContext = nullptr;

    pAudioFormatContext = avformat_alloc_context();
    if(pAudioFormatContext==nullptr){
        throw runtime_error("Error: paudiocontext");
    }
    if ( av_dict_set(&audioOptions, "sample_rate", "44100", 0) < 0) {
        throw runtime_error("Error: cannot set audio sample rate");
    }

    if (av_dict_set(&audioOptions, "async", "1", 0) < 0) {
        throw runtime_error("Error: cannot set audio sample rate");
    }



#if defined linux
    pAudioInputFormat = av_find_input_format("alsa");



    if (avformat_open_input(&pAudioFormatContext, "default", pAudioInputFormat, &audioOptions) != 0) {
        throw runtime_error("Error in opening input device (audio)");
    }
#elif defined _WIN32

    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    if (value != 0) {
        throw runtime_error("Error in opening input device (audio)");
    }

#else



    pAudioInputFormat = av_find_input_format("avfoundation");

    if (avformat_open_input(&pAudioFormatContext, ":0", pAudioInputFormat, &audioOptions) != 0) {
        throw runtime_error("Error in opening input device (audio)");
    }

#endif

    if (avformat_find_stream_info(pAudioFormatContext, nullptr) != 0) {
        throw runtime_error("Error: cannot find the audio stream information");
    }

    for (int i = 0; i < pAudioFormatContext->nb_streams; i++) {
        if (pAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        throw logic_error("Error: unable to find audio stream index");
    }



    AVCodecParameters* params = pAudioFormatContext->streams[audioStreamIndx]->codecpar;

    pAudioCodec = avcodec_find_decoder(params->codec_id);
    if (pAudioCodec == nullptr) {
        throw runtime_error("Error: cannot find the audio decoder");
    }

    pAudioCodecContext = avcodec_alloc_context3(pAudioCodec);
    if (avcodec_parameters_to_context(pAudioCodecContext, params) < 0) {
        throw runtime_error("Cannot create codec context for audio input");
    }

    if (avcodec_open2(pAudioCodecContext, pAudioCodec, nullptr) < 0) {
        throw runtime_error("Error: cannot open the input audio codec");
    }

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        throw runtime_error("Error: cannot create audio stream");
    }

    outAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (outAudioCodec == nullptr) {
        throw runtime_error("Error: cannot find requested encoder");
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        throw runtime_error("Error: cannot create related VideoCodecContext");
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
        throw runtime_error("Error in opening the avcodec");
    }

    //find a free stream index
    for (i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outAudioStreamIndex = i;\
        }
    }
    if (outAudioStreamIndex < 0) {
        throw logic_error("Error: cannot find a free stream for audio on the output");
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);

    return 0;
}

int ScreenCapture::openInput(int widthi, int heighti, string outputi,bool audioi, string xi, string yi) {


    string co;
    width= widthi;
    height= heighti;
    co = to_string(width) + "x" + to_string(height);
    conc = co.c_str();
    audio=audioi;
    string o="../"+outputi+".mp4";
    output=o.c_str();

    string s;
    s = "+"+xi+","+yi;
    string lin;
    lin = ":0.0" + s;
    linSta = lin.c_str();
    string win;
    win = "descktop" + s;
    winSta = win.c_str();
    sta = s.c_str();
    x=xi;
    y=yi;

    openInputVideo();

     if(audio){
        openInputAudio();
    }


    streamTrail();

    return 0;
}

int ScreenCapture::streamTrail(){


    if(avformat_write_header(outAVFormatContext , &options) < 0)
    {
        throw runtime_error("Error in writing the header context");
    }


    av_dump_format(outAVFormatContext , 0 ,output ,1);
}

void ScreenCapture::pause_recording(){
    pause=true;
}

void ScreenCapture::resume_recording(){
    pause=false;
    cv_v.notify_all();
    cv_a.notify_all();

}

void ScreenCapture::terminate_recording(){
    running=false;
    captureVideo_thread.get()->join();
    if(audio){
        captureAudio_thread.get()->join();

    }

    if (av_write_trailer(outAVFormatContext) < 0) {
        throw runtime_error("Error in writing av trailer");

    }

    avformat_close_input(&pAVFormatContext);
    if( pAVFormatContext != nullptr ) {
        throw runtime_error("Unable to close the file");
    }



    if(audio) {

        avformat_close_input(&pAudioFormatContext);
        if(pAudioFormatContext != nullptr){
            throw runtime_error("Error: unable to close the file audio");
        }

        avcodec_free_context(&pAudioCodecContext);
        if(pAudioCodecContext != nullptr){
            throw runtime_error("Error: unable to close the file audio");
        }
    }


    avformat_close_input(&outAVFormatContext);
    if(outAVFormatContext != nullptr){
        throw runtime_error("Error: unable to close the file");
    }

}
