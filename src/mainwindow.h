#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "floppydiskwidget.h"
#include "fdccontrollerwidget.h"

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
    void onPlayPauseClicked();
    void onResetClicked();
    void onSpeedChanged(int index);
    void updateAnimation();

private:
    Ui::MainWindow *ui;
    FloppyDiskWidget *floppyWidget;
    FDCControllerWidget *fdcWidget;
    QTimer *animationTimer;
    bool isPlaying;
    double currentSpeed;
    
    void setupUI();
    void createConnections();
};
#endif // MAINWINDOW_H 