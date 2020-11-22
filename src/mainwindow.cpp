#include "mainwindow.h"
#include "ui_mainwindow.h"

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
