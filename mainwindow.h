#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "md3colors.h"

class NetworkClient;
class VideoWidget;
class TimelineWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    VideoWidget *videoWidget;
    TimelineWidget *timelineWidget;
    NetworkClient *networkClient;
};
#endif // MAINWINDOW_H
