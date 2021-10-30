//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"

using namespace std;

ScreenCapture::ScreenCapture(const char* start){

    avdevice_register_all();
    pAVFormatContext = NULL;

    //Context allocation
    pAVFormatContext = avformat_alloc_context();
    //Set screen as input device
    pAVInputFormat = av_find_input_format("x11grab");
    if(avformat_open_input(&pAVFormatContext, start, pAVInputFormat, NULL) != 0) {
        cout<<"Error in opening the input device!";
        exit(1);
    }

    cout<<"Required functions registered successfully";
}

ScreenCapture::~ScreenCapture(){



}

