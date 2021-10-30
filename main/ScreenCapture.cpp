//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"

using namespace std;

ScreenCapture::ScreenCapture(){

    avdevice_register_all();
    cout<<"\nall required functions are registered successfully";

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

    if(avformat_open_input(&pAVFormatContext, ":0.0", pAVInputFormat, NULL) != 0) {
        cout<<"Error in opening the input device!";
        exit(1);
    }

  /*  if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(2);
    }*/


/*    if(avformat_find_stream_info(pAVFormatContext,NULL) < 0) da fare forse per il pausa e riprendi
    {
        cout<<"\nunable to find the stream information";
        exit(1);
    }*/


}
