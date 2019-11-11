#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <windows.h>
#include <QString>
#include <dbt.h>
#include <QDebug>
#include<devguid.h>
#include<SetupAPI.h>
#include<InitGuid.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void GetPort();
    ~MainWindow();
private slots:
        void ReadData();//读取数据

        void timeTosend();//定时发送

        void on_openButton_clicked();

        void on_r_clearButton_clicked();

        void on_s_clearButton_clicked();

        void on_sendButton_clicked();

        void StringToHex(QString str, QByteArray & senddata);//用于发送时进制转换

        char ConvertHexChart(char ch);//十六进制转换

        void readToHex();//将读取的数据以十六进制显示

        void Mdisplay();

private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    QTimer *time;           //用于定时发送
 static   int sendBytes;
 static   int receBytes;
};

#endif // MAINWINDOW_H
