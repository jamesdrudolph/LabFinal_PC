#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QThread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnConnect_clicked()
{
    if (!connected) {
        connect();
    } else {
        disconnect();
    }
}

void MainWindow::on_btnSendOverlay_clicked()
{
    if (openFileDialog(testImagePath)) {
        QImage qImage = QImage(testImagePath);
        QPixmap qPixmap = QPixmap::fromImage(qImage);
        transfer_image(testImagePath, "IMAGE");
    }
}

bool MainWindow::openFileDialog(QString &filePath) {
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Bitmap Images (*.bmp)"));

    if (dialog.exec() && dialog.selectedFiles().length() > 0) {
        filePath = dialog.selectedFiles()[0];
        return true;
    }

    return false;
}

void MainWindow::on_sliderContrastValue_valueChanged(int value)
{
    //update gui with new contrast value
    ui->lblContrastValue->setText("Contrast: " + QString::number(value));
    //send contrast value and specifier to fpga
    sendValue(ui->sliderContrastValue->value(), "CONTRAST");
}

void MainWindow::on_sliderBrightnessValue_valueChanged(int value)
{
    //update gui with new brightness value
    ui->lblBrightnessValue->setText("Brightness: " + QString::number(value));
    //send brightness value and specifier to fpga
    sendValue(ui->sliderBrightnessValue->value(), "BRIGHTNESS");
}

void MainWindow::sendValue(int value, QString valueType) {
    //convert to data type supported by write()
    QString strToSend = QString::number(value);

    //prepend data type specifier and send
    strToSend = valueType + ":" + strToSend;
    udpSocket->write(strToSend.toStdString().c_str());
}

void MainWindow::connect() {
    //save gui elements as variables
    int port = ui->sbPort->value();
    QString ip = ui->txtIP->text();

    //connect to fpga
    udpSocket->connectToHost(ip, port);

    bool connectSuccess = false;
    connectSuccess = udpSocket->waitForConnected(1000); //waits 1000 ms, returns t/f;

    if (connectSuccess) {
        connected = true;
        ui->btnConnect->setText("Disconnect");
        toggleEnabledControls();
    }
}

void MainWindow::disconnect() {
    //send disconnect message and disconnect
    udpSocket->write("BYE");
    udpSocket->disconnectFromHost();

    //disconnect from fpga
    bool disconnectSuccess = false;
    disconnectSuccess = (udpSocket->state() == QAbstractSocket::UnconnectedState || udpSocket->waitForDisconnected(1000));

    if (disconnectSuccess) {
        connected = false;
        ui->btnConnect->setText("Connect");
        toggleEnabledControls();
    }
}

void MainWindow::toggleEnabledControls() {
    //toggle enabled of send controls
    toggleWidgetEnabled(ui->btnSendOverlay);
    toggleWidgetEnabled(ui->lblBrightnessValue);
    toggleWidgetEnabled(ui->lblContrastValue);
    toggleWidgetEnabled(ui->sliderBrightnessValue);
    toggleWidgetEnabled(ui->sliderContrastValue);

    //toggle enabled of connect controls
    toggleWidgetEnabled(ui->lblIP);
    toggleWidgetEnabled(ui->lblPort);
    toggleWidgetEnabled(ui->txtIP);
    toggleWidgetEnabled(ui->sbPort);
}

void MainWindow::toggleWidgetEnabled(QWidget *qWidget) {
    //toggle enabled of QWidget
    qWidget->setEnabled(!qWidget->isEnabled());
}

void MainWindow::transfer_image(QString bmpFilePath, QByteArray image)
{
    //std::ifstream is (bmpFilePath.toStdString().c_str(), std::ifstream::binary);
    std::ifstream is (bmpFilePath.toStdString().c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    //get 1024 bytes of the file, send over UDP

    if (is) {
      udpSocket->write("START_"+image+"_SEND");
      QThread::msleep(2); //sleeping after each packet makes it work on remote server
                          //likely not the ideal way but w/e
      int fileSize = is.tellg();
      int remainder = fileSize % 1024;
      qDebug() << "\nFileSize: " << fileSize << " Remainder: " << remainder;
      is.seekg (0, std::ios::beg);
      char * buffer = new char [1024];
      if (fileSize <= 1024)
      {
          buffer = new char [fileSize];
          is.read (buffer, fileSize);
          udpSocket->write(buffer,fileSize);
          QThread::msleep(2);
      }
      else
      {
        for(int x = 0; x < int((fileSize - remainder) / 1024); x++)
        {
            buffer = new char [1024];
            is.read (buffer, 1024);
            udpSocket->write(buffer,1024);
            QThread::msleep(2);
            qDebug() << x;

        }
        if(remainder > 0)
        {
            buffer = new char [remainder];
            is.read (buffer, remainder);
            udpSocket->write(buffer,remainder);
            QThread::msleep(2);
        }
      }
      while(udpSocket->hasPendingDatagrams()){}
      udpSocket->write("END_"+image+"_SEND");
      QThread::msleep(2);
      qDebug() << "Done";
    }
}
