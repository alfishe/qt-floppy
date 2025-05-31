#include "floppydiskwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPainterPath>
#include <QtMath>

namespace {
constexpr qreal INDEX_HOLE_RADIAL_PCT = 0.286; // 28.6%
constexpr qreal ENVELOPE_INDEX_HOLE_RADIUS_PCT = 0.024; // 4.8% (diameter 4.8%, so radius 2.4%)
constexpr qreal DISK_INDEX_HOLE_RADIUS_PCT = 0.0095; // 1.9% (diameter 1.9%, so radius 0.95%)
constexpr qreal INDEX_HOLE_ANGLE_DEG = 30.0;
}

FloppyDiskWidget::FloppyDiskWidget(QWidget *parent)
    : QWidget(parent)
    , currentTrack(0)
    , currentSide(0)
    , headPosition(0)
    , isWriteOperation(false)
    , isDoubleSided(true)
    , isHighDensity(true)
    , rotationAngle(0)
    , indexPulseActive(false)
    , m_envelopeTransparency(1.0)
    , m_sectorCount(1)
{
    setMinimumSize(400, 400);
}

FloppyDiskWidget::~FloppyDiskWidget()
{
}

void FloppyDiskWidget::setTrack(int track)
{
    currentTrack = track;
    update();
}

void FloppyDiskWidget::setSide(int side)
{
    currentSide = side;
    update();
}

void FloppyDiskWidget::setHeadPosition(int position)
{
    headPosition = position;
    update();
}

void FloppyDiskWidget::setOperation(bool isWrite)
{
    isWriteOperation = isWrite;
    update();
}

void FloppyDiskWidget::setDoubleSided(bool doubleSided)
{
    isDoubleSided = doubleSided;
    update();
}

void FloppyDiskWidget::setHighDensity(bool highDensity)
{
    isHighDensity = highDensity;
    update();
}

void FloppyDiskWidget::setRotationAngle(double angle)
{
    rotationAngle = angle;
    update();
}

void FloppyDiskWidget::setIndexPulse(bool active)
{
    indexPulseActive = active;
    update();
}

void FloppyDiskWidget::setEnvelopeTransparency(qreal alpha) {
    m_envelopeTransparency = qBound(0.0, alpha, 1.0);
    update();
}

qreal FloppyDiskWidget::envelopeTransparency() const {
    return m_envelopeTransparency;
}

void FloppyDiskWidget::setSectorCount(int count) {
    m_sectorCount = (count > 0) ? count : 1;
    update();
}

int FloppyDiskWidget::sectorCount() const {
    return m_sectorCount;
}

void FloppyDiskWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Calculate the largest centered square
    int margin = 20;
    int side = qMin(width(), height()) - 2 * margin;
    int x0 = (width() - side) / 2;
    int y0 = (height() - side) / 2;
    QRectF floppyRect(x0, y0, side, side);

    // Draw the envelope (jacket) with correct holes and notches
    drawEnvelope(painter, floppyRect);

    // Draw overlays (tracks, sectors, head, status)
    drawTracks(painter, floppyRect);
    drawSectors(painter, floppyRect);
    drawHead(painter, floppyRect);
    drawStatus(painter);
}

void FloppyDiskWidget::drawEnvelope(QPainter &painter, const QRectF& envelopeRect)
{
    qreal radius = 16.0;
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25; // 1 unit = 1 inch

    // --- Envelope base ---
    QPainterPath envelopePath;
    envelopePath.addRoundedRect(envelopeRect, radius, radius);

    // --- Center hub hole ---
    qreal hubRadius = scale * 1.0 / 2.0; // 1.0" diameter
    envelopePath.addEllipse(center, hubRadius, hubRadius);

    // --- Index hole (window) ---
    qreal indexHoleAngleRad = qDegreesToRadians(INDEX_HOLE_ANGLE_DEG);
    qreal indexHoleRadialDist = envelopeRect.width() * INDEX_HOLE_RADIAL_PCT;
    QPointF indexHoleCenter = center + QPointF(indexHoleRadialDist * qCos(indexHoleAngleRad),
                                               indexHoleRadialDist * qSin(indexHoleAngleRad));
    qreal envelopeIndexHoleRadius = envelopeRect.width() * ENVELOPE_INDEX_HOLE_RADIUS_PCT;
    envelopePath.addEllipse(indexHoleCenter, envelopeIndexHoleRadius, envelopeIndexHoleRadius);

    // --- Read/Write window (vertical rounded rect) ---
    qreal rwWidth = scale * 0.5;
    qreal rwHeight = scale * 1.1;
    qreal rwX = center.x() - rwWidth/2;
    qreal rwY = envelopeRect.bottom() - scale * 0.25 - rwHeight;
    QRectF rwRect(rwX, rwY, rwWidth, rwHeight);
    envelopePath.addRoundedRect(rwRect, rwWidth/2, rwWidth/2);

    // --- Write-protect notch (right edge, small rect) ---
    qreal wpWidth = scale * 0.25;
    qreal wpHeight = scale * 0.25;
    qreal wpX = envelopeRect.right() - wpWidth;
    qreal wpY = envelopeRect.top() + scale * 0.5;
    QRectF wpRect(wpX, wpY, wpWidth, wpHeight);
    envelopePath.addRect(wpRect);

    // --- Draw envelope with transparency ---
    QColor plasticColor(60, 60, 80, int(m_envelopeTransparency * 255));
    painter.setBrush(plasticColor);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawPath(envelopePath);

    // --- Draw outlines for wireframe effect ---
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
    painter.drawRoundedRect(envelopeRect, radius, radius);
    painter.drawEllipse(center, hubRadius, hubRadius);
    painter.drawEllipse(indexHoleCenter, envelopeIndexHoleRadius, envelopeIndexHoleRadius);
    painter.drawRoundedRect(rwRect, rwWidth/2, rwWidth/2);
    painter.drawRect(wpRect);

    // --- Mask for disk: only visible through center hole and read/write window ---
    QPainterPath diskMask;
    diskMask.addEllipse(center, hubRadius, hubRadius);
    diskMask.addRoundedRect(rwRect, rwWidth/2, rwWidth/2);
    painter.save();
    painter.setClipPath(diskMask);
    drawDisk(painter, envelopeRect);
    painter.restore();
}

void FloppyDiskWidget::drawDisk(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    qreal diskRadius = scale * 2.5; // 5" diameter

    // Draw disk
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::black));
    painter.drawEllipse(center, diskRadius, diskRadius);

    // --- Draw index hole on disk (rotating) ---
    qreal diskIndexHoleRadialDist = envelopeRect.width() * INDEX_HOLE_RADIAL_PCT;
    qreal diskIndexHoleBaseAngleRad = qDegreesToRadians(INDEX_HOLE_ANGLE_DEG);
    qreal diskIndexHoleEffectiveAngleRad = diskIndexHoleBaseAngleRad + qDegreesToRadians(rotationAngle);
    QPointF diskIndexHoleCenter = center + QPointF(diskIndexHoleRadialDist * qCos(diskIndexHoleEffectiveAngleRad),
                                                   diskIndexHoleRadialDist * qSin(diskIndexHoleEffectiveAngleRad));
    qreal diskIndexHoleRadius = envelopeRect.width() * DISK_INDEX_HOLE_RADIUS_PCT;
    painter.setBrush(QBrush(Qt::white));
    painter.drawEllipse(diskIndexHoleCenter, diskIndexHoleRadius, diskIndexHoleRadius);
}

void FloppyDiskWidget::drawTracks(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    int maxRadius = envelopeRect.width() * 0.48;
    int minRadius = maxRadius / 3;
    int numTracks = isHighDensity ? 80 : 40;
    painter.setPen(QPen(Qt::blue, 1));
    for (int i = 0; i < numTracks; i++) {
        double trackRadius = minRadius + (maxRadius - minRadius) * i / (numTracks - 1);
        painter.drawEllipse(center, trackRadius, trackRadius);
    }
}

void FloppyDiskWidget::drawSectors(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    qreal maxRadius = envelopeRect.width() * 0.48;
    painter.setPen(QPen(Qt::red, 1));
    int sectors = m_sectorCount;
    for (int i = 1; i < sectors; i++) {
        double angleDeg = i * (360.0 / sectors) + rotationAngle;
        double angleRad = qDegreesToRadians(angleDeg);
        painter.drawLine(center,
                        QPointF(center.x() + maxRadius * std::cos(angleRad),
                                center.y() + maxRadius * std::sin(angleRad)));
    }
}

void FloppyDiskWidget::drawHead(QPainter &painter, const QRectF& envelopeRect)
{
    // Read/write window geometry (must match drawEnvelope)
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    qreal rwWidth = scale * 0.5;
    qreal rwHeight = scale * 1.1;
    qreal rwX = center.x() - rwWidth/2;
    qreal rwY = envelopeRect.bottom() - scale * 0.25 - rwHeight;
    QRectF rwRect(rwX, rwY, rwWidth, rwHeight);

    // Head position: moves vertically within rwRect based on currentTrack
    int numTracks = isHighDensity ? 80 : 40;
    qreal frac = (numTracks > 1) ? (qreal)currentTrack / (numTracks - 1) : 0.0;
    qreal headY = rwRect.top() + frac * rwRect.height();
    qreal headX = rwRect.center().x();

    // Plastic mount (semi-transparent rectangle)
    qreal mountWidth = rwWidth * 0.95;
    qreal mountHeight = rwWidth * 0.38;
    QRectF mountRect(headX - mountWidth/2, headY - mountHeight/2, mountWidth, mountHeight);
    QColor mountColor(120, 180, 255, 90); // Light blue, semi-transparent
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(mountColor);
    painter.drawRect(mountRect);

    // Head size and shape (oval metallic)
    qreal headWidth = rwWidth * 0.7;
    qreal headHeight = rwWidth * 0.28;
    QRectF headRect(headX - headWidth/2, headY - headHeight/2, headWidth, headHeight);
    qreal headRadius = headHeight * 0.4;

    // Metallic gradient
    QLinearGradient grad(headRect.topLeft(), headRect.bottomRight());
    grad.setColorAt(0, QColor(220, 220, 220));
    grad.setColorAt(0.5, QColor(180, 180, 180));
    grad.setColorAt(1, QColor(120, 120, 120));

    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(grad);
    painter.drawRoundedRect(headRect, headRadius, headRadius);

    // Draw the pad/slot in the center
    QRectF padRect(headX - headWidth*0.12, headY - headHeight*0.18, headWidth*0.24, headHeight*0.36);
    painter.setBrush(QColor(60, 60, 60));
    painter.drawRoundedRect(padRect, headRadius*0.5, headRadius*0.5);

    // Optional: highlight for write operation
    if (isWriteOperation) {
        painter.setBrush(QColor(255, 0, 0, 120));
        painter.drawRoundedRect(headRect, headRadius, headRadius);
    }
}

void FloppyDiskWidget::drawStatus(QPainter &painter)
{
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    
    QString status = QString("Track: %1  Side: %2  %3  %4")
        .arg(currentTrack)
        .arg(currentSide)
        .arg(isWriteOperation ? "Write" : "Read")
        .arg(isHighDensity ? "HD" : "DD");
    
    painter.drawText(10, height() - 10, status);
}

QSize FloppyDiskWidget::sizeHint() const
{
    return QSize(400, 400);
} 