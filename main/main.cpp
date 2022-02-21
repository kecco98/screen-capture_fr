#include "ScreenCapture.h"
#include "cstring"

using namespace std;

/* driver function to run the application */
int main()
{
    const char* output;
    const char* conc;
    int width, height;
    bool aud;
    string in2, co, x, y, au;
    cout<<"Inserire x!"<<endl;
    cin >> x;
    cout<<"Inserire y"<<endl;
    cin >> y;
    ScreenCapture video_record;
    cout<<"Insert the output patch!"<<endl;
    cin >> in2;
    in2="../media_output/"+in2+".mp4";
    output=in2.c_str();
    cout<<"Insert the width of the window you want to record!"<<endl;
    cin >> width;
    cout<<"Insert the height of the window you want to record!"<<endl;
    cin >> height;
    co = to_string(width) + "x" + to_string(height);
    conc = co.c_str();
    cout<< conc;
    cout<<"Press S or N if you want to register also the Audio"<<endl;
    cin >> au;

    if (au == "S") {
        aud=true;
    } else {
        aud=false;
    }

    video_record.openInput(width, height,output,aud, x, y);
    video_record.start();

    cout<<"\nProgram executed successfully"<<endl;

    return 0;
}