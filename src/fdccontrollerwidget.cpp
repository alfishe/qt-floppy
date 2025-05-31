#include "fdccontrollerwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>

FDCControllerWidget::FDCControllerWidget(QWidget *parent)
    : QWidget(parent)
    , statusReg(0)
    , commandReg(0)
    , trackReg(0)
    , sectorReg(0)
    , dataReg(0)
    , interruptActive(false)
    , dataRequestActive(false)
{
    setMinimumSize(300, 200);
}

FDCControllerWidget::~FDCControllerWidget()
{
}

void FDCControllerWidget::setStatusRegister(quint8 status)
{
    statusReg = status;
    update();
}

void FDCControllerWidget::setCommandRegister(quint8 command)
{
    commandReg = command;
    update();
}

void FDCControllerWidget::setTrackRegister(quint8 track)
{
    trackReg = track;
    update();
}

void FDCControllerWidget::setSectorRegister(quint8 sector)
{
    sectorReg = sector;
    update();
}

void FDCControllerWidget::setDataRegister(quint8 data)
{
    dataReg = data;
    update();
}

void FDCControllerWidget::setInterruptStatus(bool active)
{
    interruptActive = active;
    update();
}

void FDCControllerWidget::setDataRequest(bool active)
{
    dataRequestActive = active;
    update();
}

void FDCControllerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), Qt::white);
    painter.setPen(QPen(Qt::black, 1));

    // Draw title
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(10, 20, "WD1793 FDC Status");

    // Draw registers
    int y = 40;
    drawRegister(painter, "Status", statusReg, 10, y);
    drawRegister(painter, "Command", commandReg, 10, y + 30);
    drawRegister(painter, "Track", trackReg, 10, y + 60);
    drawRegister(painter, "Sector", sectorReg, 10, y + 90);
    drawRegister(painter, "Data", dataReg, 10, y + 120);

    // Draw status indicators
    drawStatus(painter, "INT", interruptActive, 200, y);
    drawStatus(painter, "DRQ", dataRequestActive, 200, y + 30);
}

void FDCControllerWidget::drawRegister(QPainter &painter, const QString &name, quint8 value, int x, int y)
{
    painter.setFont(QFont("Arial", 10));
    painter.drawText(x, y, name + ":");
    painter.drawText(x + 80, y, formatBinary(value));
    painter.drawText(x + 200, y, QString("0x%1").arg(value, 2, 16, QChar('0')));
}

void FDCControllerWidget::drawStatus(QPainter &painter, const QString &name, bool active, int x, int y)
{
    painter.setFont(QFont("Arial", 10));
    painter.drawText(x, y, name + ":");
    
    QColor color = active ? Qt::red : Qt::gray;
    painter.setPen(QPen(color, 1));
    painter.setBrush(QBrush(color));
    painter.drawEllipse(x + 40, y - 5, 10, 10);
}

QString FDCControllerWidget::formatBinary(quint8 value)
{
    QString binary;
    for (int i = 7; i >= 0; --i) {
        binary += (value & (1 << i)) ? "1" : "0";
    }
    return binary;
}

QSize FDCControllerWidget::sizeHint() const
{
    return QSize(300, 200);
} 