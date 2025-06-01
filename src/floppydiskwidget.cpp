#include "floppydiskwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPainterPath>
#include <QtMath>

namespace {
// Based on the blueprint, index hole should be closer to the center
constexpr qreal INDEX_HOLE_RADIAL_PCT = 0.13; // 13% - closer to center as per blueprint
constexpr qreal ENVELOPE_INDEX_HOLE_RADIUS_PCT = 0.018; // Smaller index hole (3.6% diameter)
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
    , isDoubleDensity(true)
    , rotationAngle(0)
    , indexPulseActive(false)
    , m_envelopeTransparency(1.0)
    , m_sectorCount(16)  // Default to 16 sectors for standard floppy
    , m_currentSector(0)
    , m_highlightTrack(true)
    , m_highlightSector(true)
    , m_animationTimer(new QTimer(this))
    , m_sideAnimationTimer(new QTimer(this))
    , m_isHeadAnimating(false)
    , m_animationStep(0)
    , m_animationDirectionUp(true)
    , m_animationSpeed(1.0)
{
    setMinimumSize(400, 400);
    
    // Connect the animation timer to the animation slot
    connect(m_animationTimer, &QTimer::timeout, this, &FloppyDiskWidget::animateHead);
    
    // Connect the side animation timer - updates twice as fast as track movement
    connect(m_sideAnimationTimer, &QTimer::timeout, this, &FloppyDiskWidget::animateSide);
}

FloppyDiskWidget::~FloppyDiskWidget()
{
    if (m_animationTimer) {
        m_animationTimer->stop();
        delete m_animationTimer;
        m_animationTimer = nullptr;
    }
    
    if (m_sideAnimationTimer) {
        m_sideAnimationTimer->stop();
        delete m_sideAnimationTimer;
        m_sideAnimationTimer = nullptr;
    }
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

void FloppyDiskWidget::startHeadAnimation()
{
    if (!m_isHeadAnimating) {
        m_isHeadAnimating = true;
        
        // Calculate timer intervals based on animation speed
        // Base speed (1.0x): 1 second per step for track movement
        int trackInterval = static_cast<int>(1000 / m_animationSpeed);
        int sideInterval = trackInterval / 2; // Side changes twice as fast
        
        m_animationTimer->start(trackInterval);
        m_sideAnimationTimer->start(sideInterval);
    }
}

void FloppyDiskWidget::stopHeadAnimation()
{
    if (m_isHeadAnimating) {
        m_isHeadAnimating = false;
        m_animationTimer->stop();
        m_sideAnimationTimer->stop();
        // Don't reset animation position - it should resume from where it stopped
    }
}

void FloppyDiskWidget::resetHeadAnimation()
{
    m_isHeadAnimating = false;
    m_animationTimer->stop();
    m_sideAnimationTimer->stop();
    m_animationStep = 0;
    m_animationDirectionUp = true;
    currentTrack = 0;
    currentSide = 0;
    update();
}

bool FloppyDiskWidget::isHeadAnimating() const
{
    return m_isHeadAnimating;
}

void FloppyDiskWidget::setAnimationSpeed(qreal speed)
{
    if (speed > 0) {
        m_animationSpeed = speed;
        
        // If animation is running, update the timers with new intervals
        if (m_isHeadAnimating) {
            int trackInterval = static_cast<int>(1000 / m_animationSpeed);
            int sideInterval = trackInterval / 2;
            
            m_animationTimer->setInterval(trackInterval);
            m_sideAnimationTimer->setInterval(sideInterval);
        }
    }
}

void FloppyDiskWidget::setHighlightTrack(bool highlight)
{
    if (m_highlightTrack != highlight) {
        m_highlightTrack = highlight;
        update();
    }
}

void FloppyDiskWidget::setHighlightSector(bool highlight)
{
    if (m_highlightSector != highlight) {
        m_highlightSector = highlight;
        update();
    }
}

void FloppyDiskWidget::setCurrentSector(int sector)
{
    if (sector >= 0 && sector < m_sectorCount && m_currentSector != sector) {
        m_currentSector = sector;
        update();
    }
}

int FloppyDiskWidget::getCurrentSector() const
{
    return m_currentSector;
}

int FloppyDiskWidget::getSectorCount() const
{
    return m_sectorCount;
}

void FloppyDiskWidget::animateSide()
{
    if (!m_isHeadAnimating) {
        return;
    }
    
    // Toggle between side 0 and side 1 if double-sided
    if (isDoubleSided) {
        currentSide = (currentSide == 0) ? 1 : 0;
        update();
    }
}

void FloppyDiskWidget::animateHead()
{
    if (!m_isHeadAnimating) {
        return;
    }

    // Update step counter based on direction
    if (m_animationDirectionUp) {
        m_animationStep++;
        if (m_animationStep >= ANIMATION_STEPS - 1) {
            m_animationDirectionUp = false;
        }
    } else {
        m_animationStep--;
        if (m_animationStep <= 0) {
            m_animationDirectionUp = true;
        }
    }

    // Update track indicator based on animation step
    // Map animation step (0 to ANIMATION_STEPS-1) to track number (0 to 79 for HD)
    int numTracks = isDoubleDensity ? 80 : 40;
    int newTrack = static_cast<int>((static_cast<float>(m_animationStep) / (ANIMATION_STEPS - 1)) * (numTracks - 1));
    if (newTrack != currentTrack) {
        currentTrack = newTrack;
    }

    // Calculate current sector based on rotation angle
    int sectorCount = m_sectorCount;
    int currentSector = static_cast<int>(fmod(rotationAngle, 360.0) / (360.0 / sectorCount));
    setCurrentSector(currentSector);

    // Trigger repaint
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

void FloppyDiskWidget::setDoubleDensity(bool doubleDensity)
{
    isDoubleDensity = doubleDensity;
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

    // Draw overlays in correct order for proper visibility
    // First draw tracks without highlighting
    drawTracks(painter, floppyRect);
    
    // Then draw sector highlighting on top of tracks
    drawSectors(painter, floppyRect);
    
    // Finally draw the head
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
    qreal rwWidth = scale * 0.45; // Narrower window
    qreal rwHeight = scale * 1.7; // Tall window
    qreal rwX = center.x() - rwWidth/2;
    qreal rwY = envelopeRect.bottom() - scale * 0.20 - rwHeight;
    QRectF rwRect(rwX, rwY, rwWidth, rwHeight);
    envelopePath.addRoundedRect(rwRect, rwWidth/2, rwWidth/2);

    // --- Write-protect notch (right edge, small rect) ---
    qreal wpWidth = scale * 0.25;
    qreal wpHeight = scale * 0.25;
    qreal wpX = envelopeRect.right() - wpWidth;
    qreal wpY = envelopeRect.top() + scale * 0.5;
    QRectF wpRect(wpX, wpY, wpWidth, wpHeight);
    
    // Create a cutout in the envelope path for the write-protect notch
    QPainterPath wpNotchPath;
    wpNotchPath.addRect(wpRect);
    envelopePath = envelopePath.subtracted(wpNotchPath);
    
    // --- Add insertion guide cutouts at the bottom (rounded triangles) ---
    qreal guideWidth = scale * 0.3;
    qreal guideHeight = scale * 0.08;
    qreal guideSpacing = scale * 0.85; // Space between the two guides
    
    // Left guide
    QPainterPath leftGuidePath;
    QPointF leftGuideCenter(center.x() - guideSpacing/2, envelopeRect.bottom());
    QPolygonF leftGuidePolygon;
    leftGuidePolygon << QPointF(leftGuideCenter.x() - guideWidth/2, envelopeRect.bottom())
                     << QPointF(leftGuideCenter.x() + guideWidth/2, envelopeRect.bottom())
                     << QPointF(leftGuideCenter.x(), envelopeRect.bottom() - guideHeight);
    leftGuidePath.addPolygon(leftGuidePolygon);
    
    // Right guide
    QPainterPath rightGuidePath;
    QPointF rightGuideCenter(center.x() + guideSpacing/2, envelopeRect.bottom());
    QPolygonF rightGuidePolygon;
    rightGuidePolygon << QPointF(rightGuideCenter.x() - guideWidth/2, envelopeRect.bottom())
                      << QPointF(rightGuideCenter.x() + guideWidth/2, envelopeRect.bottom())
                      << QPointF(rightGuideCenter.x(), envelopeRect.bottom() - guideHeight);
    rightGuidePath.addPolygon(rightGuidePolygon);
    
    // Subtract both guides from the envelope path
    envelopePath = envelopePath.subtracted(leftGuidePath);
    envelopePath = envelopePath.subtracted(rightGuidePath);
    
    // --- Draw envelope with transparency ---
    QColor plasticColor(60, 60, 80, int(m_envelopeTransparency * 255));
    painter.setBrush(plasticColor);
    painter.setPen(QPen(Qt::black, 2));
    
    // Draw the main envelope path with the write-protect notch cutout
    painter.drawPath(envelopePath);

    // --- Draw outlines for wireframe effect ---
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
    
    // Create a custom outline path for the envelope that excludes the write-protect notch
    QPainterPath outlinePath;
    
    // Start with a rounded rectangle for the envelope
    QPainterPath envelopeOutline;
    envelopeOutline.addRoundedRect(envelopeRect, radius, radius);
    
    // Create a path for the write-protect notch area (slightly larger to ensure no artifacts)
    QPainterPath wpOutlineExclude;
    qreal wpOutlineMargin = 1.0; // Small margin to ensure clean exclusion
    QRectF wpOutlineRect = wpRect.adjusted(-wpOutlineMargin, -wpOutlineMargin, wpOutlineMargin, wpOutlineMargin);
    wpOutlineExclude.addRect(wpOutlineRect);
    
    // Subtract the write-protect notch from the envelope outline
    outlinePath = envelopeOutline.subtracted(wpOutlineExclude);
    
    // Draw the custom envelope outline
    painter.drawPath(outlinePath);
    
    // Draw other outlines
    painter.drawEllipse(center, hubRadius, hubRadius);
    
    // Draw index hole with grayish color
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(QColor(160, 160, 160))); // Grayish color
    painter.drawEllipse(indexHoleCenter, envelopeIndexHoleRadius, envelopeIndexHoleRadius);
    
    // Reset brush and draw read/write window
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rwRect, rwWidth/2, rwWidth/2);

    // --- Mask for disk: only visible through center hole, index hole, and read/write window ---
    QPainterPath diskMask;
    diskMask.addEllipse(center, hubRadius, hubRadius);
    diskMask.addEllipse(indexHoleCenter, envelopeIndexHoleRadius, envelopeIndexHoleRadius);
    diskMask.addRoundedRect(rwRect, rwWidth/2, rwWidth/2);
    painter.save();
    painter.setClipPath(diskMask);
    drawDisk(painter, envelopeRect);
    
    // Draw sector lines visible through the holes and write-protect notch
    QPainterPath sectorMask = diskMask;
    
    // Add the write-protect notch to the sector mask so sectors are visible there too
    sectorMask.addRect(wpRect);
    
    // Create disk area as the whole disk circle
    qreal diskRadius = scale * 2.5; // 5" diameter
    
    QPainterPath diskArea;
    diskArea.addEllipse(center, diskRadius, diskRadius);
    
    // Set the clip path to the intersection of the sector mask and full disk area
    painter.setClipPath(sectorMask.intersected(diskArea));
    drawSectors(painter, envelopeRect);
    painter.restore();
}

void FloppyDiskWidget::drawDisk(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    qreal diskRadius = scale * 2.5; // 5" diameter

    // Calculate disk index hole parameters - use same position as envelope index hole but apply rotation
    qreal diskIndexHoleRadialDist = envelopeRect.width() * INDEX_HOLE_RADIAL_PCT;
    qreal diskIndexHoleBaseAngleRad = qDegreesToRadians(INDEX_HOLE_ANGLE_DEG);
    qreal diskIndexHoleEffectiveAngleRad = diskIndexHoleBaseAngleRad + qDegreesToRadians(rotationAngle);
    QPointF diskIndexHoleCenter = center + QPointF(diskIndexHoleRadialDist * qCos(diskIndexHoleEffectiveAngleRad),
                                                  diskIndexHoleRadialDist * qSin(diskIndexHoleEffectiveAngleRad));
    qreal diskIndexHoleRadius = envelopeRect.width() * DISK_INDEX_HOLE_RADIUS_PCT;
    
    // Create disk path with index hole cutout
    QPainterPath diskPath;
    diskPath.addEllipse(center, diskRadius, diskRadius);
    
    // Create index hole path
    QPainterPath indexHolePath;
    indexHolePath.addEllipse(diskIndexHoleCenter, diskIndexHoleRadius, diskIndexHoleRadius);
    
    // Subtract index hole from disk path
    QPainterPath finalDiskPath = diskPath.subtracted(indexHolePath);
    
    // Draw disk with index hole cutout
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(Qt::black));
    painter.drawPath(finalDiskPath);
}

void FloppyDiskWidget::drawTracks(QPainter &painter, const QRectF& envelopeRect)
{
    // Draw concentric circles representing tracks
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    
    // Start tracks further from the hub to match screenshot
    qreal hubRadius = scale * 0.5; // 1.0" diameter hub
    
    // Shift all tracks 10% closer to the center
    qreal minRadius = scale * 0.8; // Decreased by 10% to shift tracks closer to center
    qreal maxRadius = scale * 2.1; // Also decreased by ~10% to maintain proportions
    int numTracks = isDoubleDensity ? 80 : 40;
    // Increase track spacing by 20% to cover more area within the magnetic disk
    qreal trackSpacing = 1.2 * ((maxRadius - minRadius) / numTracks);
    
    painter.save();
    
    // Draw all tracks with blue/violet color (restore original color)
    painter.setPen(QPen(QColor(100, 100, 255, 120), scale * 0.01));
    for (int i = 0; i <= numTracks; i++) {
        qreal radius = minRadius + i * trackSpacing;
        painter.drawEllipse(center, radius, radius);
    }
    
    // Highlight current track if enabled - fill the entire track with green
    if (m_highlightTrack && currentTrack >= 0 && currentTrack < numTracks) {
        // Calculate inner and outer radius of the current track
        qreal currentTrackIndex = numTracks - currentTrack;
        
        // Store these values in class fields for use in other methods
        m_trackInnerRadius = minRadius + (currentTrackIndex - 0.5) * trackSpacing;
        m_trackOuterRadius = minRadius + (currentTrackIndex + 0.5) * trackSpacing;
        
        // Create a path for the track ring
        QPainterPath trackPath;
        trackPath.addEllipse(center, m_trackOuterRadius, m_trackOuterRadius);
        trackPath.addEllipse(center, m_trackInnerRadius, m_trackInnerRadius);
        
        // Fill the track with semi-transparent green
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 200, 0, 80)); // Semi-transparent green
        painter.drawPath(trackPath);
        
        // Draw track outline
        qreal currentRadius = minRadius + currentTrackIndex * trackSpacing;
        painter.setPen(QPen(QColor(0, 200, 0, 180), scale * 0.02));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, currentRadius, currentRadius);
    }
    
    painter.restore();
}

void FloppyDiskWidget::drawSectorBoundaries(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    
    // Sector lines should start at the same radius as tracks
    qreal minRadius = scale * 0.8; // Match track starting radius in drawTracks
    qreal maxRadius = scale * 2.1; // Match outer edge in drawTracks
    
    // Calculate index hole position (for reference)
    qreal indexHoleAngleDeg = INDEX_HOLE_ANGLE_DEG;
    
    painter.save();
    
    int sectors = m_sectorCount;
    qreal sectorAngle = 360.0 / sectors;
    
    // Draw all sector boundary lines
    for (int i = 0; i < sectors; i++) {
        double angleDeg = indexHoleAngleDeg + i * sectorAngle + rotationAngle;
        double angleRad = qDegreesToRadians(angleDeg);
        QPointF innerPoint(center.x() + minRadius * qCos(angleRad),
                          center.y() + minRadius * qSin(angleRad));
        QPointF outerPoint(center.x() + maxRadius * qCos(angleRad),
                          center.y() + maxRadius * qSin(angleRad));
        
        // Draw sector line
        painter.setPen(QPen(QColor(255, 0, 0, 100), 1));
        painter.drawLine(innerPoint, outerPoint);
    }
    
    painter.restore();
}

void FloppyDiskWidget::drawHighlightedSector(QPainter &painter, const QRectF& envelopeRect)
{
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    
    // Sector lines should start at the same radius as tracks
    qreal minRadius = scale * 0.8; // Match track starting radius in drawTracks
    qreal maxRadius = scale * 2.1; // Match outer edge in drawTracks
    
    // Calculate index hole position (for reference)
    qreal indexHoleAngleDeg = INDEX_HOLE_ANGLE_DEG;
    
    int sectors = m_sectorCount;
    qreal sectorAngle = 360.0 / sectors;
    
    // Only draw the highlighted sector if enabled and valid sector
    if (m_highlightSector && m_currentSector >= 0 && m_currentSector < sectors) {
        painter.save();
        
        // Use the track radius values that were calculated and stored in drawTracks
        // This ensures perfect synchronization between track highlighting and sector highlighting
        qreal innerRadius = m_trackInnerRadius;
        qreal outerRadius = m_trackOuterRadius;
        
        // Calculate the sector that should be highlighted under the head
        // This makes the sector static below the head
        
        // Find the angle of the head position (90 degrees = top of disk)
        qreal headAngleDeg = 90.0;
        
        // Calculate which sector is currently under the head
        // First, normalize the rotation angle to 0-360 degrees
        qreal normalizedRotation = fmod(rotationAngle, 360.0);
        if (normalizedRotation < 0) normalizedRotation += 360.0;
        
        // Calculate the absolute angle (disk coordinates)
        // For clockwise rotation, we add the rotation angle instead of subtracting
        qreal absoluteAngle = headAngleDeg + normalizedRotation;
        if (absoluteAngle >= 360.0) absoluteAngle -= 360.0;
        
        // Determine which sector is under the head
        int headSector = static_cast<int>(fmod(absoluteAngle - indexHoleAngleDeg + 360.0, 360.0) / sectorAngle);
        if (headSector < 0) headSector += sectors;
        if (headSector >= sectors) headSector -= sectors;
        
        // Override the current sector with the one under the head
        int sectorToHighlight = headSector;
        
        // Calculate the start angle for the sector under the head
        // For clockwise rotation, we need to invert the rotation angle
        // This makes the sector rotate clockwise with the disk
        qreal startAngle = indexHoleAngleDeg + sectorToHighlight * sectorAngle - rotationAngle;
        
        // Create a completely clean sector shape without ANY parasitic lines
        // Instead of creating a closed path, we'll draw two separate arcs and fill the area between them
        
        // Create a clip path for the sector
        QPainterPath clipPath;
        
        // Add two radial lines to define the sector boundaries
        clipPath.moveTo(center);
        clipPath.lineTo(center.x() + maxRadius * qCos(qDegreesToRadians(startAngle)),
                        center.y() + maxRadius * qSin(qDegreesToRadians(startAngle)));
        clipPath.lineTo(center.x() + maxRadius * qCos(qDegreesToRadians(startAngle + sectorAngle)),
                        center.y() + maxRadius * qSin(qDegreesToRadians(startAngle + sectorAngle)));
        clipPath.closeSubpath();
        
        // Save painter state before applying clip
        painter.save();
        
        // Apply the sector clip path
        painter.setClipPath(clipPath);
        
        // Now draw a ring that will be clipped to the sector shape
        QPainterPath ringPath;
        ringPath.addEllipse(center, outerRadius, outerRadius);
        ringPath.addEllipse(center, innerRadius, innerRadius);
        
        // Fill the ring with semi-transparent red
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 0, 0, 120)); // Semi-transparent red
        painter.drawPath(ringPath);
        
        // Restore painter state
        painter.restore();
        
        painter.restore();
    }
}

void FloppyDiskWidget::drawSectors(QPainter &painter, const QRectF& envelopeRect)
{
    // Draw sector boundary lines first
    drawSectorBoundaries(painter, envelopeRect);
    
    // Then draw the highlighted sector on top
    drawHighlightedSector(painter, envelopeRect);
}

void FloppyDiskWidget::drawHead(QPainter &painter, const QRectF& envelopeRect)
{
    // Read/write window geometry (must match drawEnvelope)
    QPointF center = envelopeRect.center();
    qreal scale = envelopeRect.width() / 5.25;
    qreal rwWidth = scale * 0.45; // Match the current narrower window width
    qreal rwHeight = scale * 1.7; // Match the current window height in drawEnvelope
    qreal rwX = center.x() - rwWidth/2;
    // Shifted 5% lower to match drawEnvelope
    qreal rwY = envelopeRect.bottom() - scale * 0.15 - rwHeight;
    QRectF rwRect(rwX, rwY, rwWidth, rwHeight);

    // Calculate head position based on animation or track
    qreal headX = rwRect.center().x();
    qreal headY;
    
    // Determine the current track (either from animation or direct track value)
    int numTracks = isDoubleDensity ? 80 : 40;
    int mappedTrack;
    
    if (m_isHeadAnimating) {
        // When animating, use the animation step to determine position
        qreal normalizedStep = (qreal)m_animationStep / (ANIMATION_STEPS - 1);
        mappedTrack = static_cast<int>(normalizedStep * (numTracks - 1));
    } else {
        mappedTrack = currentTrack;
    }
    
    // Invert the position calculation so track 0 is at the bottom (outermost)
    qreal trackFraction = (numTracks > 1) ? (qreal)mappedTrack / (numTracks - 1) : 0.0;
    
    // If we're on the same track as the highlighted track and highlighting is enabled,
    // use the stored track radius values for perfect alignment
    qreal trackRadius;
    
    if (m_highlightTrack && mappedTrack == currentTrack) {
        // Use the center radius of the highlighted track (average of inner and outer)
        trackRadius = (m_trackInnerRadius + m_trackOuterRadius) / 2.0;
    } else {
        // Otherwise, calculate the radius as before
        qreal minRadius = scale * 0.8;  // Match track starting radius in drawTracks
        qreal maxRadius = scale * 2.1;  // Match outer edge in drawTracks
        // Increase track spacing by 20% to cover more area within the magnetic disk
        qreal trackSpacing = 1.2 * ((maxRadius - minRadius) / numTracks);
        
        // Calculate the track radius for the current track
        // Use the same formula as in drawTracks but invert the track numbering
        trackRadius = minRadius + (numTracks - mappedTrack) * trackSpacing;
        
        // For track 0, we need to position the head exactly at the outermost track
        if (mappedTrack == 0) {
            // For track 0, use the exact radius of the outermost track
            trackRadius = maxRadius;
        }
    }
    
    // Calculate the intersection point of this track with the vertical line at 90 degrees (top)
    qreal trackY = center.y() + trackRadius * qSin(qDegreesToRadians(90.0));
    
    // Position the head at this exact Y coordinate
    headY = trackY;
    
    // This positions the head at the top of the disk, matching the orientation
    // of the center hole and correcting the head position

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
    
    // Include the sector number in the status display
    QString status = QString("Track: %1  Side: %2  Sector: %3  %4  %5")
        .arg(currentTrack)
        .arg(currentSide)
        .arg(m_currentSector)
        .arg(isWriteOperation ? "Write" : "Read")
        .arg(isDoubleDensity ? "DD" : "SD");
    
    painter.drawText(10, height() - 10, status);
}

QSize FloppyDiskWidget::sizeHint() const
{
    return QSize(400, 400);
} 