//
// Created by kecco on 30/10/21.
//

#include "ScreenCapture.h"

using namespace std;

/* driver function to run the application */
int main()
{
    char start[4];
    cout<<"Insert the starting point!"<<endl;
    cin >>start;
    ScreenCapture video_record;

    video_record.setup(start);

    cout<<"Program executed successfully"<<endl;

    return 0;
}