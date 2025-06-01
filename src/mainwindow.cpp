#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , isPlaying(false)
    , currentSpeed(1.0)
{
    ui->setupUi(this);
    createConnections();

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);
    animationTimer->start(16); // ~60 FPS
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    // No manual widget/layout creation. All handled by .ui file.
}

void MainWindow::createConnections()
{
    // Connect control buttons
    connect(findChild<QPushButton*>("Play/Pause"), &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);
    connect(findChild<QPushButton*>("Stop"), &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(findChild<QPushButton*>("Reset"), &QPushButton::clicked, this, &MainWindow::onResetClicked);
    
    // Connect speed control
    connect(findChild<QComboBox*>(), QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                currentSpeed = findChild<QComboBox*>()->itemData(index).toDouble();
            });
}

void MainWindow::onPlayPauseClicked()
{
    isPlaying = !isPlaying;
    if (isPlaying) {
        animationTimer->start();
    } else {
        animationTimer->stop();
    }
}

void MainWindow::onStopClicked()
{
    isPlaying = false;
    animationTimer->stop();
    // Reset animation state
    ui->floppyWidget->setRotationAngle(0);
    ui->floppyWidget->setIndexPulse(false);
}

void MainWindow::onResetClicked()
{
    onStopClicked();
    // Reset all states
    ui->floppyWidget->setTrack(0);
    ui->floppyWidget->setSide(0);
    ui->floppyWidget->setHeadPosition(0);
    ui->floppyWidget->setOperation(false);
    ui->floppyWidget->setDoubleSided(true);
    ui->floppyWidget->setHighDensity(true);
}

void MainWindow::onSpeedChanged(double speed)
{
    currentSpeed = speed;
}

void MainWindow::updateAnimation()
{
    static double angle = 0;
    static int indexPulseCounter = 0;
    
    if (isPlaying) {
        angle += 0.1 * currentSpeed;
        if (angle >= 360) {
            angle = 0;
        }
        
        // Simulate index pulse (once per revolution)
        indexPulseCounter++;
        if (indexPulseCounter >= 360) {
            indexPulseCounter = 0;
            ui->floppyWidget->setIndexPulse(true);
        } else if (indexPulseCounter == 10) {
            ui->floppyWidget->setIndexPulse(false);
        }
        
        ui->floppyWidget->setRotationAngle(angle);
        ui->floppyWidget->update();
    }
} 