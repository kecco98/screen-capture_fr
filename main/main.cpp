//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"
#include "cstring"

using namespace std;

/* driver function to run the application */
int main()
{
    const char* start;
    const char* output;
    string in1, in2;
    cout<<"Insert the starting point!"<<endl;
    cin >>in1;
    in1=":"+in1;
    start=in1.c_str();
    ScreenCapture video_record;
    cout<<"Insert the output patch!"<<endl;
    cin >> in2;
    output=in2.c_str();


    video_record.setup(start, output);

    cout<<"Program executed successfully"<<endl;

    return 0;
}