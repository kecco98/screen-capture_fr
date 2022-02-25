#include "ScreenCapture.h"


using namespace std;

/* Main function to run the application */
int main() {

    int width, height;
    bool aud, ok = false;
    ScreenCapture video_record;
    string in2, co, x, y, au, output;
    char p;
    char r;
    bool running =true;
    bool pause=false;

    //Taking screen starting point parameters
    cout << "Inserire x!" << endl;
    cin >> x;
    cout << "Inserire y" << endl;
    cin >> y;

    //Taking output file name
    cout << "Insert the output patch!" << endl;
    cin >> output;

    //Taking width and heigth parameters
    cout << "Insert the width of the window you want to record!" << endl;
    cin >> width;
    cout << "Insert the height of the window you want to record!" << endl;
    cin >> height;
    co = to_string(width) + "x" + to_string(height);



    //Taking audio parameters
    while (ok == false) {
    cout << "Press Y or N if you want to register also the audio " << endl;
    cin >> au;
    if (au == "Y" || au == "y") {
        aud = true;
        ok=true;
    } else if (au == "N" || au == "n") {
        aud = false;
        ok=true;
    }
       }

    try{
        auto open_thread = thread{
                [&]() {
                    video_record.openInput(width, height,output,aud, x, y);
                    video_record.start();
                }};



        cout<<"Is recording!"<<endl;
        cout<<"- write p to pause the recording"<<endl;
        cout<<"- write t to terminate the recording"<<endl;

        while(running){
            cin>>p;
            if(p=='p'){
                video_record.pause_recording();
                pause=true;
                cout<<"Recording in pause!"<<endl;
                cout<<"- write r to restart the recording"<<endl;
                while(pause){
                    cin>>r;
                    if(r=='r'){
                        video_record.resume_recording();
                        pause=false;
                        cout<<"Recording restarted!"<<endl;
                    }
                }
            } else if(p=='t'){
                running=false;
                video_record.terminate_recording();
                cout<<endl<<"Recording terminated"<<endl;
            }
        }


        open_thread.join();

    } catch (const std::exception& e) {
        std::cerr << e.what() << endl;
        cout << "There was an error in the library" << endl;
        video_record.terminate_recording();
    }

    return 0;
}