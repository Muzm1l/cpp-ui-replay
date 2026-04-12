#ifndef SCALEVIEW_H
#define SCALEVIEW_H

#include <QWidget>
#include <QTime>
#include <QVector>
#include <QStringList>
#include <QPoint>
#include <QRectF>

class ScaleView : public QWidget
{
    Q_OBJECT

public:
    explicit ScaleView(QWidget *parent = nullptr);

    void setTimeRange(const QTime &startTime, const QTime &endTime);
    void setMarkers(const QVector<QTime> &markerTimes, const QStringList &markerLabels = QStringList());
    void setHighlightedTime(const QTime &time);
    void clearHighlightedTime();
    void setSelectionRange(const QTime &startTime, const QTime &endTime);
    void clearSelectionRange();

    QSize sizeHint() const override;

signals:
    void highlightedTimeChanged(const QTime &time);
    void selectionRangeChanged(const QTime &startTime, const QTime &endTime);
    void timeHorizonChanged(const QString &duration);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    enum class DragMode {
        None,
        CreateRange,
        ResizeLeft,
        ResizeRight
    };

    struct Marker
    {
        QTime time;
        QString label;
    };

    QTime rangeStart;
    QTime rangeEnd;
    QVector<Marker> markers;
    QTime highlightedTime;
    bool hasHighlightedTime;
    QTime selectionStart;
    QTime selectionEnd;
    bool hasSelectionRange;
    DragMode dragMode;
    bool dragMoved;
    QPoint dragStartPos;
    QTime dragAnchorTime;
    static constexpr int minSelectionSeconds = 1;

    QTime xToTime(int x, const QRect &rect) const;
    int timeToSeconds(const QTime &time) const;
    QTime normalizedTime(const QTime &time) const;
    void setHighlightedTimeInternal(const QTime &time, bool emitSignal);
    void setSelectionRangeInternal(const QTime &startTime, const QTime &endTime, bool emitSignal);
    QRectF selectionRectForContent(const QRect &content) const;
    bool nearLeftHandle(const QPoint &pos, const QRect &content) const;
    bool nearRightHandle(const QPoint &pos, const QRect &content) const;

    QRect contentRect() const;
    double timeToX(const QTime &time, const QRect &rect) const;
    QString formatTimeLabel(const QTime &time) const;
    QString formatDurationLabel(int seconds) const;
};

#endif // SCALEVIEW_H