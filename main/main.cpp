#include "ScreenCapture.h"
#include "cstring"

using namespace std;

/* Main function to run the application */
int main()
{
    const char* conc;
    int width, height;
    bool aud;
    ScreenCapture video_record;
    string in2, co, x, y, au, output;

    //Taking screen starting point parameters
    cout<<"Inserire x!"<<endl;
    cin >> x;
    cout<<"Inserire y"<<endl;
    cin >> y;

    //Taking output file name
    cout<<"Insert the output patch!"<<endl;
    cin >> output;

    //Taking width and heigth parameters
    cout<<"Insert the width of the window you want to record!"<<endl;
    cin >> width;
    cout<<"Insert the height of the window you want to record!"<<endl;
    cin >> height;
    co = to_string(width) + "x" + to_string(height);
    conc = co.c_str();
    cout<< conc;

    //Taking audio parameters
    cout<<"Press S or N if you want to register also the audio"<<endl;
    cin >> au;
    if (au == "S" || au == "s") {
        aud=true;
    } else {
        aud=false;
    }

    //Starting running two threads, one fo the settings and one for the menu.
    try{
        auto open_thread = thread{
                [&]() {
                    video_record.openInput(width, height,output,aud, x, y);
                    video_record.start();
                }};
        auto menu_thread = thread{
                [&]() {
                        video_record.genMenu();
                }};
        open_thread.join();
        menu_thread.join();
    } catch (const std::exception& e) {
        std::cerr << e.what() << endl;
        cout << "There was an error in the library" << endl;
        video_record.terminate_recording();
    }

    return 0;
}