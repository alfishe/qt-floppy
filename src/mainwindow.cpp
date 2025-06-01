#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), isPlaying(false), currentSpeed(1.0) {
    ui->setupUi(this);
    createConnections();

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimation);
    animationTimer->start(16); // ~60 FPS
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupUI() {
    // No manual widget/layout creation. All handled by .ui file.
}

void MainWindow::createConnections() {
    // Connect toolbar actions
    connect(ui->actionPlay, &QAction::triggered, this, &MainWindow::onPlayPauseClicked);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onResetClicked);

    // Connect speed combo box
    connect(ui->speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSpeedChanged);

    // Set default speed to 1x (index 5)
    ui->speedComboBox->setCurrentIndex(5);
}

void MainWindow::onPlayPauseClicked() {
    isPlaying = !isPlaying;
    if (isPlaying) {
        animationTimer->start();
        // Start the head animation when play is clicked
        ui->floppyWidget->startHeadAnimation();
    } else {
        animationTimer->stop();
        // Stop the head animation when pause is clicked
        ui->floppyWidget->stopHeadAnimation();
    }
}

void MainWindow::onResetClicked() {
    // Stop all animations
    isPlaying = false;
    animationTimer->stop();
    ui->floppyWidget->stopHeadAnimation();

    // Reset all states
    ui->floppyWidget->setTrack(0);
    ui->floppyWidget->setSide(0);
    ui->floppyWidget->setHeadPosition(0);
    ui->floppyWidget->setOperation(false);
    ui->floppyWidget->setDoubleSided(true);
    ui->floppyWidget->setDoubleDensity(true);
}

void MainWindow::onSpeedChanged(int index) {
    // Map combo box index to speed multiplier
    const QMap<int, double> speedMap = {
            {0, 0.01},  // 0.01x speed (ultra slow)
            {1, 0.05},  // 0.05x speed (very slow)
            {2, 0.1},   // 0.1x speed
            {3, 0.25},  // 0.25x speed
            {4, 0.5},   // 0.5x speed
            {5, 1.0},   // 1x speed
            {6, 2.0}    // 2x speed
    };

    currentSpeed = speedMap.value(index, 1.0);

    // Update the animation speed in the floppy widget
    ui->floppyWidget->setAnimationSpeed(currentSpeed);
}

void MainWindow::updateAnimation() {
    static double angle = 0;
    static int indexPulseCounter = 0;

    if (isPlaying) {
        // Standard floppy disk rotates at 300 RPM = 5 RPS = 1800 degrees per second
        // At 60 FPS (16ms interval), that's 30 degrees per frame at 1x speed
        double angleIncrement = 30.0 * currentSpeed;

        angle += angleIncrement;
        if (angle >= 360) {
            angle = fmod(angle, 360.0); // Wrap around properly
        }

        // Calculate current sector based on rotation angle
        int sectorCount = ui->floppyWidget->getSectorCount();
        int currentSector = static_cast<int>(fmod(angle, 360.0) / (360.0 / sectorCount));
        ui->floppyWidget->setCurrentSector(currentSector);

        // Simulate index pulse (once per revolution)
        indexPulseCounter++;
        if (indexPulseCounter >= 360 / angleIncrement) {
            indexPulseCounter = 0;
            ui->floppyWidget->setIndexPulse(true);
        } else if (indexPulseCounter == 2) { // Short pulse
            ui->floppyWidget->setIndexPulse(false);
        }

        ui->floppyWidget->setRotationAngle(angle);
        ui->floppyWidget->update();
    }
} 