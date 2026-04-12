#include "scaleview.h"

#include <QMouseEvent>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <algorithm>

namespace {
constexpr int kLeftMargin = 14;
constexpr int kRightMargin = 14;
constexpr int kTopMargin = 4;
constexpr int kBottomMargin = 12;
constexpr int kAxisBottomGap = 6;
constexpr int kHandleHitTolerance = 8;
}

ScaleView::ScaleView(QWidget *parent)
    : QWidget(parent)
    , rangeStart(QTime(0, 0, 0))
    , rangeEnd(QTime(0, 10, 0))
    , highlightedTime()
    , hasHighlightedTime(false)
    , selectionStart()
    , selectionEnd()
    , hasSelectionRange(false)
    , dragMode(DragMode::None)
    , dragMoved(false)
    , dragStartPos()
    , dragAnchorTime()
{
    setMinimumHeight(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setMouseTracking(true);
}

void ScaleView::setTimeRange(const QTime &startTime, const QTime &endTime)
{
    rangeStart = startTime.isValid() ? startTime : QTime(0, 0, 0);
    rangeEnd = endTime.isValid() ? endTime : QTime(0, 10, 0);
    if (rangeEnd <= rangeStart) {
        rangeEnd = rangeStart.addSecs(60);
    }

    if (hasSelectionRange) {
        setSelectionRangeInternal(selectionStart, selectionEnd, false);
    }
    update();
}

void ScaleView::setMarkers(const QVector<QTime> &markerTimes, const QStringList &markerLabels)
{
    markers.clear();
    markers.reserve(markerTimes.size());

    for (int index = 0; index < markerTimes.size(); ++index) {
        Marker marker;
        marker.time = markerTimes.at(index);
        if (index < markerLabels.size()) {
            marker.label = markerLabels.at(index);
        }
        markers.push_back(marker);
    }

    update();
}

void ScaleView::setHighlightedTime(const QTime &time)
{
    setHighlightedTimeInternal(time, false);
}

void ScaleView::clearHighlightedTime()
{
    setHighlightedTimeInternal(QTime(), false);
}

void ScaleView::setSelectionRange(const QTime &startTime, const QTime &endTime)
{
    setSelectionRangeInternal(startTime, endTime, false);
}

void ScaleView::clearSelectionRange()
{
    setSelectionRangeInternal(QTime(), QTime(), false);
}

QSize ScaleView::sizeHint() const
{
    return QSize(720, 96);
}

QRect ScaleView::contentRect() const
{
    return rect().adjusted(kLeftMargin, kTopMargin, -kRightMargin, -kBottomMargin);
}

double ScaleView::timeToX(const QTime &time, const QRect &content) const
{
    const int rangeSeconds = qMax(1, rangeStart.secsTo(rangeEnd));
    const int clampedSeconds = qBound(0, rangeStart.secsTo(time), rangeSeconds);
    const double ratio = static_cast<double>(clampedSeconds) / static_cast<double>(rangeSeconds);
    return content.left() + (ratio * content.width());
}

QString ScaleView::formatTimeLabel(const QTime &time) const
{
    return time.isValid() ? time.toString("hh:mm:ss") : QString();
}

QString ScaleView::formatDurationLabel(int seconds) const
{
    const int hours = seconds / 3600;
    const int minutes = (seconds % 3600) / 60;
    const int remainingSeconds = seconds % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(remainingSeconds, 2, 10, QLatin1Char('0'));
}

int ScaleView::timeToSeconds(const QTime &time) const
{
    return time.isValid() ? QTime(0, 0, 0).secsTo(time) : 0;
}

QTime ScaleView::normalizedTime(const QTime &time) const
{
    if (!time.isValid()) {
        return rangeStart;
    }
    const int startSeconds = timeToSeconds(rangeStart);
    const int endSeconds = timeToSeconds(rangeEnd);
    const int valueSeconds = qBound(startSeconds, timeToSeconds(time), endSeconds);
    return QTime(0, 0, 0).addSecs(valueSeconds);
}

QTime ScaleView::xToTime(int x, const QRect &rect) const
{
    const int rangeSeconds = qMax(1, rangeStart.secsTo(rangeEnd));
    const int clampedX = qBound(rect.left(), x, rect.right());
    const double ratio = static_cast<double>(clampedX - rect.left()) / static_cast<double>(qMax(1, rect.width()));
    const int offsetSeconds = static_cast<int>(qRound(ratio * rangeSeconds));
    return rangeStart.addSecs(offsetSeconds);
}

void ScaleView::setHighlightedTimeInternal(const QTime &time, bool emitSignal)
{
    const QTime normalized = time.isValid() ? normalizedTime(time) : QTime();
    const bool changed = (hasHighlightedTime != normalized.isValid()) || (highlightedTime != normalized);
    highlightedTime = normalized;
    hasHighlightedTime = normalized.isValid();
    update();
    if (changed && emitSignal) {
        emit highlightedTimeChanged(highlightedTime);
    }
}

void ScaleView::setSelectionRangeInternal(const QTime &startTime, const QTime &endTime, bool emitSignal)
{
    const bool valid = startTime.isValid() && endTime.isValid() && endTime > startTime;
    const QTime normalizedStart = valid ? normalizedTime(startTime) : QTime();
    const QTime normalizedEnd = valid ? normalizedTime(endTime) : QTime();
    const bool changed = (hasSelectionRange != valid) || (selectionStart != normalizedStart) || (selectionEnd != normalizedEnd);
    selectionStart = normalizedStart;
    selectionEnd = normalizedEnd;
    hasSelectionRange = valid;
    update();
    if (changed && emitSignal && hasSelectionRange) {
        emit selectionRangeChanged(selectionStart, selectionEnd);
        emit timeHorizonChanged(formatDurationLabel(selectionStart.secsTo(selectionEnd)));
    }

    if (!hasSelectionRange && emitSignal) {
        emit timeHorizonChanged(QString());
    }
}

QRectF ScaleView::selectionRectForContent(const QRect &content) const
{
    if (!hasSelectionRange || !selectionStart.isValid() || !selectionEnd.isValid()) {
        return QRectF();
    }

    const int availableHeight = qMax(80, content.height());
    const int axisY = content.bottom() - kAxisBottomGap;
    const int selectionHeight = qBound(18, availableHeight / 3, 30);
    const int selectionTopOffset = qBound(12, availableHeight / 4, 20);
    const double selectionLeft = timeToX(selectionStart, content);
    const double selectionRight = timeToX(selectionEnd, content);

    QRectF rect(selectionLeft,
                axisY - selectionTopOffset,
                selectionRight - selectionLeft,
                selectionHeight);
    return rect.normalized();
}

bool ScaleView::nearLeftHandle(const QPoint &pos, const QRect &content) const
{
    const QRectF selectionRect = selectionRectForContent(content);
    if (!selectionRect.isValid()) {
        return false;
    }

    const QRectF leftHandle(selectionRect.left() - kHandleHitTolerance,
                            selectionRect.top() - kHandleHitTolerance,
                            kHandleHitTolerance * 2,
                            selectionRect.height() + (kHandleHitTolerance * 2));
    return leftHandle.contains(pos);
}

bool ScaleView::nearRightHandle(const QPoint &pos, const QRect &content) const
{
    const QRectF selectionRect = selectionRectForContent(content);
    if (!selectionRect.isValid()) {
        return false;
    }

    const QRectF rightHandle(selectionRect.right() - kHandleHitTolerance,
                             selectionRect.top() - kHandleHitTolerance,
                             kHandleHitTolerance * 2,
                             selectionRect.height() + (kHandleHitTolerance * 2));
    return rightHandle.contains(pos);
}

void ScaleView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient bgGradient(rect().topLeft(), rect().topRight());
    bgGradient.setColorAt(0.0, QColor(20, 22, 25));
    bgGradient.setColorAt(0.35, QColor(36, 39, 44));
    bgGradient.setColorAt(0.7, QColor(30, 33, 38));
    bgGradient.setColorAt(1.0, QColor(18, 20, 23));
    painter.fillRect(rect(), bgGradient);

    const QRect content = contentRect();
    if (!content.isValid()) {
        return;
    }

    QPen borderPen(QColor(190, 190, 190, 180));
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    const int availableHeight = qMax(80, content.height());
    const int headerHeight = qBound(18, availableHeight / 4, 24);
    const int labelBaselineY = content.top() + headerHeight + qBound(20, availableHeight / 3, 30);
    const int axisY = content.bottom() - kAxisBottomGap;
    const int tickHeight = qBound(10, availableHeight / 4, 18);

    const QString horizonText = hasSelectionRange
        ? formatDurationLabel(selectionStart.secsTo(selectionEnd))
        : formatDurationLabel(rangeStart.secsTo(rangeEnd));
    const QString title = QStringLiteral("Time Range : %1").arg(horizonText);

    QFont headerFont = font();
    headerFont.setPointSize(qBound(9, availableHeight / 10, 13));
    headerFont.setBold(false);
    painter.setFont(headerFont);
    painter.setPen(QColor(240, 240, 240));

    const QString startLabel = formatTimeLabel(rangeStart);
    const QString endLabel = formatTimeLabel(rangeEnd);
    painter.drawText(QRectF(content.left() + 4, content.top(), 170, headerHeight), Qt::AlignLeft | Qt::AlignVCenter, startLabel);
    painter.drawText(QRectF(content.center().x() - 170, content.top(), 340, headerHeight), Qt::AlignCenter | Qt::AlignVCenter, title);
    painter.drawText(QRectF(content.right() - 174, content.top(), 170, headerHeight), Qt::AlignRight | Qt::AlignVCenter, endLabel);

    QPen linePen(QColor(215, 215, 215));
    linePen.setWidth(2);
    painter.setPen(linePen);
    painter.drawLine(QPointF(content.left() + 4, axisY), QPointF(content.right() - 4, axisY));

    const QRectF selectionRect = selectionRectForContent(content);
    if (selectionRect.isValid()) {
        QPen selectionPen(QColor(220, 220, 220, 210));
        selectionPen.setWidth(1);
        painter.setPen(selectionPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(selectionRect);

        QPen handlePen(QColor(220, 220, 220, 230));
        handlePen.setWidth(2);
        painter.setPen(handlePen);
        painter.drawLine(QPointF(selectionRect.left(), selectionRect.top()), QPointF(selectionRect.left(), selectionRect.bottom()));
        painter.drawLine(QPointF(selectionRect.right(), selectionRect.top()), QPointF(selectionRect.right(), selectionRect.bottom()));
    }

    const int majorTickCount = 15;
    painter.setPen(linePen);
    for (int tickIndex = 0; tickIndex <= majorTickCount; ++tickIndex) {
        const double ratio = static_cast<double>(tickIndex) / static_cast<double>(majorTickCount);
        const double x = content.left() + (ratio * content.width());
        painter.drawLine(QPointF(x, axisY), QPointF(x, axisY - tickHeight));
    }

    QFont markerFont = font();
    markerFont.setPointSize(qBound(8, availableHeight / 9, 12));
    painter.setFont(markerFont);
    const QFontMetrics markerMetrics(markerFont);

    if (!markers.isEmpty()) {
        std::vector<int> order(markers.size());
        for (int i = 0; i < markers.size(); ++i) {
            order[static_cast<std::size_t>(i)] = i;
        }

        std::sort(order.begin(), order.end(), [this](int a, int b) {
            return markers[a].time < markers[b].time;
        });

        double lastRight = content.left() - 1000.0;
        const double minGap = 2.0;

        for (int sortedIndex : order) {
            const Marker &marker = markers[sortedIndex];
            if (!marker.time.isValid()) {
                continue;
            }

            const QString labelText = marker.label.isEmpty() ? formatTimeLabel(marker.time) : marker.label;
            const int labelWidth = markerMetrics.horizontalAdvance(labelText);
            const double x = timeToX(marker.time, content);
            const double textX = x - (labelWidth / 2.0);
            const bool isHighlighted = hasHighlightedTime && marker.time == highlightedTime;

            if (!isHighlighted && textX < (lastRight + minGap)) {
                continue;
            }

            if (isHighlighted) {
                const int highlightPadX = 8;
                const int highlightPadY = 3;
                QRectF highlightRect(textX - highlightPadX,
                                     labelBaselineY - markerMetrics.ascent() - highlightPadY,
                                     labelWidth + (highlightPadX * 2),
                                     markerMetrics.height() + (highlightPadY * 2));
                QPen glowPen(QColor(0, 240, 255, 80));
                glowPen.setWidth(4);
                painter.setPen(glowPen);
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(highlightRect.adjusted(-1.0, -1.0, 1.0, 1.0), 2.0, 2.0);

                QPen highlightPen(QColor(0, 240, 255));
                highlightPen.setWidth(2);
                painter.setPen(highlightPen);
                painter.setBrush(QColor(0, 40, 50, 80));
                painter.drawRoundedRect(highlightRect, 2.0, 2.0);
                painter.setPen(QColor(0, 240, 255));
                painter.drawText(QPointF(textX, labelBaselineY), labelText);
                lastRight = std::max(lastRight, highlightRect.right());
            } else {
                painter.setPen(QColor(238, 238, 238));
                painter.drawText(QPointF(textX, labelBaselineY), labelText);
                lastRight = textX + labelWidth;
            }
        }
    }

    // Arrow at the end of the full time scale.
    const double arrowTipX = content.right() - 1.0;
    const double arrowTipY = axisY;
    QPolygonF arrow;
    arrow << QPointF(arrowTipX, arrowTipY)
          << QPointF(arrowTipX - 10.0, arrowTipY - 7.0)
          << QPointF(arrowTipX - 10.0, arrowTipY + 7.0);
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.setBrush(QColor(200, 200, 200));
    painter.drawPolygon(arrow);
}

void ScaleView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QRect content = contentRect();
    if (!content.isValid()) {
        return;
    }

    dragMoved = false;
    dragStartPos = event->pos();

    if (nearLeftHandle(event->pos(), content)) {
        dragMode = DragMode::ResizeLeft;
    } else if (nearRightHandle(event->pos(), content)) {
        dragMode = DragMode::ResizeRight;
    } else {
        dragMode = DragMode::CreateRange;
        dragAnchorTime = xToTime(event->pos().x(), content);
    }

    setHighlightedTimeInternal(xToTime(event->pos().x(), content), true);
    event->accept();
}

void ScaleView::mouseMoveEvent(QMouseEvent *event)
{
    const QRect content = contentRect();
    if (!content.isValid()) {
        return;
    }

    const QTime hoveredTime = xToTime(event->pos().x(), content);
    if ((event->buttons() & Qt::LeftButton) && dragMode != DragMode::None) {
        dragMoved = true;
        if (dragMode == DragMode::ResizeLeft && hasSelectionRange) {
            QTime newStart = normalizedTime(hoveredTime);
            QTime maxStart = selectionEnd.addSecs(-minSelectionSeconds);
            if (newStart >= maxStart) {
                newStart = maxStart;
            }
            setSelectionRangeInternal(newStart, selectionEnd, true);
        } else if (dragMode == DragMode::ResizeRight && hasSelectionRange) {
            QTime newEnd = normalizedTime(hoveredTime);
            QTime minEnd = selectionStart.addSecs(minSelectionSeconds);
            if (newEnd <= minEnd) {
                newEnd = minEnd;
            }
            setSelectionRangeInternal(selectionStart, newEnd, true);
        } else if (dragMode == DragMode::CreateRange) {
            const QTime startTime = dragAnchorTime;
            if (hoveredTime >= startTime) {
                setSelectionRangeInternal(startTime, hoveredTime, true);
            } else {
                setSelectionRangeInternal(hoveredTime, startTime, true);
            }
        }
        setHighlightedTimeInternal(hoveredTime, true);
        event->accept();
        return;
    }

    setHighlightedTimeInternal(hoveredTime, true);
    event->accept();
}

void ScaleView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (dragMode == DragMode::CreateRange && !dragMoved) {
            setSelectionRangeInternal(QTime(), QTime(), false);
        }
        dragMode = DragMode::None;
        dragMoved = false;
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void ScaleView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    if (dragMode == DragMode::None) {
        clearHighlightedTime();
    }
}