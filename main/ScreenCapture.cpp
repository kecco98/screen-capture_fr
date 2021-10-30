//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"

using namespace std;

ScreenCapture::ScreenCapture(const char* start){

    avdevice_register_all();
    pAVFormatContext = NULL;
    options = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");
    if(avformat_open_input(&pAVFormatContext, start, pAVInputFormat, NULL) != 0) {
        cout<<"Error in opening the input device!";
        exit(1);
    }

    if(av_dict_set( &options,"framerate","30",0 ) < 0)
    {
        cout<<"\nerror in setting dictionary value";
        exit(2);
    }

    cout<<"Required functions registered successfully";
}

ScreenCapture::~ScreenCapture(){



}

