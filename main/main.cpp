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
    const char* conc;
    int width, height;
    string in1, in2, co, x, y;
    cout<<"Insert the starting point!"<<endl;
    cin >> in1;
    cout<<"Inserire x!"<<endl;
    cin >> x;
    cout<<"Inserire y"<<endl;
    cin >> y;
    in1=":"+in1+"+"+x+","+y;
    start=in1.c_str();
    ScreenCapture video_record;
    cout<<"Insert the output patch!"<<endl;
    cin >> in2;
    in2="../media_output/"+in2+".mp4";
    output=in2.c_str();
    cout<<"Insert the width of the window you want to record!"<<endl;
    cin >> width;
    cout<<"Insert the height of the window you want to record!"<<endl;
    cin >> height;
    co = to_string(height) + "x" + to_string(width);
    conc = co.c_str();
    cout<< conc;

    video_record.setup(start, output, width, height, conc);
    video_record.startRecording();

    cout<<"\nProgram executed successfully"<<endl;

    return 0;
}