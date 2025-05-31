#ifndef FLOPPYDISKWIDGET_H
#define FLOPPYDISKWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>

class FloppyDiskWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FloppyDiskWidget(QWidget *parent = nullptr);
    ~FloppyDiskWidget();

    void setTrack(int track);
    void setSide(int side);
    void setHeadPosition(int position);
    void setOperation(bool isWrite);
    void setDoubleSided(bool doubleSided);
    void setHighDensity(bool highDensity);
    void setRotationAngle(double angle);
    void setIndexPulse(bool active);
    void setEnvelopeTransparency(qreal alpha);
    qreal envelopeTransparency() const;
    void setSectorCount(int count);
    int sectorCount() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    int currentTrack;
    int currentSide;
    int headPosition;
    bool isWriteOperation;
    bool isDoubleSided;
    bool isHighDensity;
    double rotationAngle;
    bool indexPulseActive;
    qreal m_envelopeTransparency = 0.5;
    int m_sectorCount = 16;

    void drawDisk(QPainter &painter, const QRectF& envelopeRect);
    void drawTracks(QPainter &painter, const QRectF& envelopeRect);
    void drawSectors(QPainter &painter, const QRectF& envelopeRect);
    void drawIndexHole(QPainter &painter);
    void drawHead(QPainter &painter, const QRectF& envelopeRect);
    void drawStatus(QPainter &painter);
    void drawEnvelope(QPainter &painter, const QRectF& envelopeRect);
};

#endif // FLOPPYDISKWIDGET_H 