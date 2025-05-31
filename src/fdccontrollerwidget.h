#ifndef FDCCONTROLLERWIDGET_H
#define FDCCONTROLLERWIDGET_H

#include <QWidget>
#include <QPainter>

class FDCControllerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FDCControllerWidget(QWidget *parent = nullptr);
    ~FDCControllerWidget();

    void setStatusRegister(quint8 status);
    void setCommandRegister(quint8 command);
    void setTrackRegister(quint8 track);
    void setSectorRegister(quint8 sector);
    void setDataRegister(quint8 data);
    void setInterruptStatus(bool active);
    void setDataRequest(bool active);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    quint8 statusReg;
    quint8 commandReg;
    quint8 trackReg;
    quint8 sectorReg;
    quint8 dataReg;
    bool interruptActive;
    bool dataRequestActive;

    void drawRegister(QPainter &painter, const QString &name, quint8 value, int x, int y);
    void drawStatus(QPainter &painter, const QString &name, bool active, int x, int y);
    QString formatBinary(quint8 value);
};

#endif // FDCCONTROLLERWIDGET_H 