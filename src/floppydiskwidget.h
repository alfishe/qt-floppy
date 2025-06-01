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
    void setDoubleDensity(bool highDensity);
    void setRotationAngle(double angle);
    void setIndexPulse(bool active);
    void setEnvelopeTransparency(qreal alpha);
    qreal envelopeTransparency() const;
    void setSectorCount(int count);
    int sectorCount() const;

    // Animation control
    void startHeadAnimation();
    void stopHeadAnimation();
    void resetHeadAnimation();
    bool isHeadAnimating() const;
    void setAnimationSpeed(qreal speed);
    
    // Track and sector highlighting
    void setHighlightTrack(bool highlight);
    void setHighlightSector(bool highlight);
    void setCurrentSector(int sector);
    int getCurrentSector() const;
    int getSectorCount() const;

private slots:
    void animateHead();
    void animateSide();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    // Animation constants
    static constexpr int ANIMATION_STEPS = 80;
    static constexpr int DISK_RPM = 300; // 300 RPM = 5 RPS

    int currentTrack;
    int currentSide;
    int headPosition;
    bool isWriteOperation;
    bool isDoubleSided;
    bool isDoubleDensity;
    qreal rotationAngle;
    bool indexPulseActive;
    qreal m_envelopeTransparency;
    int m_sectorCount;
    int m_currentSector;
    bool m_highlightTrack;
    bool m_highlightSector;
    
    // Track radius calculations - stored to avoid duplication
    qreal m_trackInnerRadius;
    qreal m_trackOuterRadius;
    
    // Track radius constants - used across all methods
    qreal m_minTrackRadius;
    qreal m_maxTrackRadius;
    
    // Animation properties
    QTimer* m_animationTimer = nullptr;
    QTimer* m_sideAnimationTimer = nullptr;
    bool m_isHeadAnimating = false;
    int m_animationStep = 0;
    bool m_animationDirectionUp = true;
    qreal m_animationSpeed;

    void drawDisk(QPainter &painter, const QRectF& envelopeRect);
    void drawTracks(QPainter &painter, const QRectF& envelopeRect);
    void drawSectors(QPainter &painter, const QRectF& envelopeRect);
    void drawSectorBoundaries(QPainter &painter, const QRectF& envelopeRect);
    void drawHighlightedSector(QPainter &painter, const QRectF& envelopeRect);
    void drawIndexHole(QPainter &painter);
    void drawHead(QPainter &painter, const QRectF& envelopeRect);
    void drawStatus(QPainter &painter);
    void drawEnvelope(QPainter &painter, const QRectF& envelopeRect);
};

#endif // FLOPPYDISKWIDGET_H 