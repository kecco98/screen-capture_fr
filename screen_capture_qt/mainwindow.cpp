#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QThread"





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
        ui->ResumeButton->hide();
           ui->pushButton_2->hide();
           ui->pushButton_3->hide();
           ui->recordImage->hide();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_altezza_valueChanged(int arg1)
{
    height=arg1;
}


void MainWindow::on_lunghezza_valueChanged(int arg1)
{
    width=arg1;
}




void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
   output = arg1.toStdString();
}

void MainWindow::on_checkBox_clicked()
{
    audio=!audio;
}


void MainWindow::on_coordx_valueChanged(int arg1)
{
    x=std::to_string(arg1);
}


void MainWindow::on_coordy_valueChanged(int arg1)
{
     y=std::to_string(arg1);
}

void MainWindow::on_pushButton_clicked()
{


    //this->video_record.openInput(width, height,output,audio, x, y);

    //this->video_record.start();

    ui->pushButton->hide();


    try{
        auto open_thread = std::thread{
                        [&]() {
                            video_record.openInput(width, height,output,audio, x, y);
                            video_record.start();
                        }};
        open_thread.detach();
           // video_record.openInput(width, height,output,audio, x, y);
           // video_record.start();
        } catch (const std::exception& e) {

        errorDialog.critical(0,"Error",QString::fromStdString(e.what()));



            video_record.terminate_recording();
            ui->pushButton->show();
            ui->ResumeButton->hide();
               ui->pushButton_2->hide();
               ui->recordImage->hide();
               ui->pushButton_3->hide();
             ui->status->setText("terminate");
        }
    ui->status->show();
    ui->status->setText("recording");

       ui->pushButton_3->show();
        ui->pushButton_2->show();
        ui->recordImage->show();
}


void MainWindow::on_pushButton_2_clicked()
{
    this->video_record.terminate_recording();
    ui->pushButton->show();
    ui->ResumeButton->hide();
       ui->pushButton_2->hide();
       ui->recordImage->hide();
       ui->pushButton_3->hide();
     ui->status->setText("terminate");

}


void MainWindow::on_ResumeButton_clicked()
{
    this->video_record.resume_recording();
     ui->status->setText("recording");
     ui->pushButton_3->show();
     ui->ResumeButton->hide();
     ui->pushButton_2->show();
     ui->recordImage->show();
}


void MainWindow::on_pushButton_3_clicked()
{
    this->video_record.pause_recording();
     ui->status->setText("pause");
     ui->ResumeButton->show();
     ui->pushButton_3->hide();
     ui->pushButton_2->hide();
     ui->recordImage->hide();
}

