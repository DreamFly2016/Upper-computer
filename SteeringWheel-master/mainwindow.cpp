#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    time = new QTimer(this);
    GetPort();

    //设置默认值
    ui->BaudBox->setCurrentIndex(7);
    ui->PortBox->setCurrentIndex(1);
    ui->StopBox->setCurrentIndex(0);
    ui->DataBox->setCurrentIndex(3);
    ui->ParityBox->setCurrentIndex(0);

    ui->sendButton->setEnabled(false);
    ui->sHexRadio->setEnabled(false);
    ui->sTextRadio->setChecked(true);
    ui->sTextRadio->setEnabled(false);
    ui->rTextRadio->setChecked(true);
    ui->reDisplay->setChecked(true);

    connect(ui->reSendCheck, &QCheckBox::stateChanged,
            this, &MainWindow::timeTosend);         //自动重发


    //此处偷懒简化调用
    connect(time, &QTimer::timeout, this, &MainWindow::on_sendButton_clicked);//

    connect(ui->rHexRadio, &QRadioButton::toggled, this, &MainWindow::Mdisplay);

}

   int MainWindow::sendBytes = 0;
   int MainWindow::receBytes = 0;


void MainWindow::GetPort()
{

    //获取可用串口
    const auto infos = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : infos)
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->PortBox->addItem(info.portName());
            //qDebug()<<info.portName();
            serial.close();
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete serialPort;
    delete time;
}



void MainWindow::ReadData()
{

    QByteArray buf;

    if (serialPort){

    buf = serialPort->readAll();
    if (!buf.isEmpty())
    {

        receBytes += buf.size();
        QString redata = QString("received:%1").arg(QString::number(receBytes));
        ui->sendlabel->setText(redata);
        QString myStrTemp = QString::fromLocal8Bit(buf); //支持中文显示
        if(ui->reDisplay->isChecked())
        {
            QString str = ui->textBrowser->toPlainText();
            str +=myStrTemp;
            ui->textBrowser->clear();
            ui->textBrowser->append(str);
        }
    }
    buf.clear();
    }
}

void MainWindow::timeTosend()
{
    if(ui->reSendCheck->isChecked())
    {
        if(time->isActive())
        {
            return;
        }
        else
        {
            int ms = ui->spinBox->value();
            time->start(ms);
        }
    }
    else
    {
        if(time->isActive())
        {
            time->stop();
        }
        else
        {
            return;
        }
    }
}

void MainWindow::on_openButton_clicked()
{
    if (ui->openButton->text() == tr("打开串口"))
    {
        serialPort = new QSerialPort;

        serialPort->setPortName(ui->PortBox->currentText());

        if(serialPort->open(QIODevice::ReadWrite))
        {
            switch (ui->BaudBox->currentIndex()) {
            case 7:
                serialPort->setBaudRate(QSerialPort::Baud115200);
                break;
            default:
                break;
            }

            switch (ui->StopBox->currentIndex()) {
            case 0:
                serialPort->setStopBits(QSerialPort::OneStop);
                break;
            default:
                break;
            }

            switch (ui->DataBox->currentIndex()) {
            case 3:
                serialPort->setDataBits(QSerialPort::Data8);
                break;
            default:
                break;
            }

            switch (ui->ParityBox->currentIndex()) {
            case 0:
                serialPort->setParity(QSerialPort::NoParity);
                break;
            default:
                break;
            }

            ui->openButton->setText(tr("关闭串口"));
            ui->PortBox->setEnabled(false);
            ui->BaudBox->setEnabled(false);
            ui->StopBox->setEnabled(false);
            ui->DataBox->setEnabled(false);
            ui->ParityBox->setEnabled(false);
            ui->sendButton->setEnabled(true);
            ui->sTextRadio->setEnabled(true);
            ui->sHexRadio->setEnabled(true);
            connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::ReadData);
//            connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readToHex);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), serialPort->errorString());
        }
    }
    else
    {
        serialPort->clear();
        serialPort->close();
        serialPort->deleteLater();

        ui->sendButton->setEnabled(false);
        ui->openButton->setText(tr("打开串口"));
        ui->PortBox->setEnabled(true);
        ui->BaudBox->setEnabled(true);
        ui->StopBox->setEnabled(true);
        ui->DataBox->setEnabled(true);
        ui->ParityBox->setEnabled(true);
        ui->sHexRadio->setEnabled(false);
        ui->sTextRadio->setEnabled(false);

    }
}

void MainWindow::on_r_clearButton_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_s_clearButton_clicked()
{
    ui->lineEdit->clear();
}

void MainWindow::on_sendButton_clicked()
{
    //Latin1是ISO-8859-1的别名，有些环境下写作Latin-1。ISO-8859-1编码是单字节编码，向下兼容ASCII
   //其编码范围是0x00-0xFF，0x00-0x7F之间完全和ASCII一致，0x80-0x9F之间是控制字符，0xA0-0xFF之间是文字符号。
//    QString test = ui->textEdit->toPlainText();
//    qDebug()<<test<<endl;
//    serialPort->write(test.toLocal8Bit());
    QString str = ui->lineEdit->text();
    if(!str.isEmpty())
    {
        auto isHexSend = ui->sHexRadio->isChecked();

        int len = str.length();
        if(len%2 == 1)
        {
            str = str.insert(len-1,'0');
        }
        QByteArray senddata;
        if(isHexSend)
        {
            StringToHex(str,senddata);
            if(serialPort->write(senddata)<0)
            {
                QMessageBox::critical(this, tr("Error"), serialPort->errorString());
            }
        }
        else
        {
            if(serialPort->write(ui->lineEdit->text().toLocal8Bit())<0)
            {
                QMessageBox::critical(this, tr("Error"), serialPort->errorString());
            }
        }

    }

}

void MainWindow::StringToHex(QString str, QByteArray &senddata)
{
    int hexdata,lowhexdata;
           int hexdatalen = 0;
           int len = str.length();
           senddata.resize(len/2);
           char lstr,hstr;
           for(int i=0; i<len; )
           {
               //char lstr,
               hstr=str[i].toLatin1();
               if(hstr == ' ')
               {
                   i++;
                   continue;
               }
               i++;
               if(i >= len)
                   break;
               lstr = str[i].toLatin1();
               hexdata = ConvertHexChart(hstr);
               lowhexdata = ConvertHexChart(lstr);
               if((hexdata == 16) || (lowhexdata == 16))
                   break;
               else
                   hexdata = hexdata*16+lowhexdata;
               i++;
               senddata[hexdatalen] = (char)hexdata;
               hexdatalen++;
           }
           senddata.resize(hexdatalen);
}

char MainWindow::ConvertHexChart(char ch)
{
    if((ch >= '0') && (ch <= '9'))
                return ch-0x30;  // 0x30 对应 ‘0’
            else if((ch >= 'A') && (ch <= 'F'))
                return ch-'A'+10;
            else if((ch >= 'a') && (ch <= 'f'))
                return ch-'a'+10;
    //        else return (-1);
    else return ch-ch;//不在0-f范围内的会发送成0

}

void MainWindow::readToHex()
{
          QByteArray temp = serialPort->readAll();
          auto isShow = ui->reDisplay->isChecked();         //接收显示？
          QDataStream out(&temp,QIODevice::ReadOnly);    //将字节数组读入
          while(!out.atEnd())
          {
                 qint8 outChar = 0;
                 out>>outChar;   //每字节填充一次，直到结束
                 //十六进制的转换
                 QString str = QString("%1").arg(outChar&0xFF,2,16,QLatin1Char('0'));
                 if(isShow){
                     ui->textBrowser->insertPlainText(str.toUpper());//大写
                     ui->textBrowser->insertPlainText(" ");//每发送两个字符后添加一个空格
                     ui->textBrowser->moveCursor(QTextCursor::End);
                 }
          }
}





void MainWindow::Mdisplay()
{

        if(ui->rHexRadio->isChecked())
        {
            disconnect(serialPort, &QSerialPort::readyRead, this, &MainWindow::ReadData);
            connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readToHex);
        }
        else
        {
            connect(serialPort, &QSerialPort::readyRead, this, &MainWindow::ReadData);
            disconnect(serialPort, &QSerialPort::readyRead, this, &MainWindow::readToHex);
        }
}
