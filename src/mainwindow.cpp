#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
    setupUI();
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
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Create widgets
    floppyWidget = new FloppyDiskWidget(this);
    fdcWidget = new FDCControllerWidget(this);

    // Create control buttons
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QPushButton *playPauseBtn = new QPushButton("Play/Pause", this);
    QPushButton *stopBtn = new QPushButton("Stop", this);
    QPushButton *resetBtn = new QPushButton("Reset", this);

    // Create speed control
    QLabel *speedLabel = new QLabel("Speed:", this);
    QComboBox *speedCombo = new QComboBox(this);
    speedCombo->addItem("0.1x", 0.1);
    speedCombo->addItem("0.25x", 0.25);
    speedCombo->addItem("0.5x", 0.5);
    speedCombo->addItem("1x", 1.0);
    speedCombo->addItem("2x", 2.0);
    speedCombo->setCurrentIndex(3); // Default to 1x
    speedCombo->setFixedWidth(100);

    // Add widgets to layouts
    controlLayout->addWidget(playPauseBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(resetBtn);
    controlLayout->addWidget(speedLabel);
    controlLayout->addWidget(speedCombo);
    controlLayout->addStretch();

    mainLayout->addWidget(floppyWidget);
    mainLayout->addWidget(fdcWidget);
    mainLayout->addLayout(controlLayout);

    // Set window properties
    setWindowTitle("Floppy Disk Emulator");
    resize(800, 600);
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
    floppyWidget->setRotationAngle(0);
    floppyWidget->setIndexPulse(false);
}

void MainWindow::onResetClicked()
{
    onStopClicked();
    // Reset all states
    floppyWidget->setTrack(0);
    floppyWidget->setSide(0);
    floppyWidget->setHeadPosition(0);
    floppyWidget->setOperation(false);
    floppyWidget->setDoubleSided(true);
    floppyWidget->setHighDensity(true);
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
            floppyWidget->setIndexPulse(true);
        } else if (indexPulseCounter == 10) {
            floppyWidget->setIndexPulse(false);
        }
        
        floppyWidget->setRotationAngle(angle);
        floppyWidget->update();
    }
} 