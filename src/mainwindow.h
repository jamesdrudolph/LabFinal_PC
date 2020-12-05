#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QFileDialog>

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
    void on_btnConnect_clicked();

    void on_btnSendOverlay_clicked();

    void on_sliderContrastValue_valueChanged(int value);

    void on_sliderBrightnessValue_valueChanged(int value);

    void on_btnToggleOverlay_clicked();

private:
    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;
    QString testImagePath;
    bool connected = false;

    void toggleEnabledControls();
    void toggleWidgetEnabled(QWidget *qWidget);
    void disconnect();
    void connect();
    void sendValue(int value, QString valueType);
    bool openFileDialog(QString &filePath);
    void transfer_image(QString bmpFilePath, QByteArray msg);
};
#endif // MAINWINDOW_H
