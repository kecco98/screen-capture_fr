#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <condition_variable>
#include <mutex>
#include "QMessageBox"
#include "ScreenCapture.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_altezza_valueChanged(int arg1);

    void on_lunghezza_valueChanged(int arg1);

    void on_coordx_valueChanged(int arg1);

    void on_coordy_valueChanged(int arg1);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_checkBox_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_ResumeButton_clicked();

    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui;

     QMessageBox errorDialog;
    int width = 120;
    int height =120;
    //std::string ou = "../prova.mp4" ;
    std::string output="prova";
    bool audio = true;
    std::string x = "0";
    std::string y = "0";
    ScreenCapture video_record ;

};
#endif // MAINWINDOW_H
