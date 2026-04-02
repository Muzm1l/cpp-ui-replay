#ifndef REPLAYSCREEN_H
#define REPLAYSCREEN_H

#include <QWidget>
#include <vector>

#include "simulator.h"

class QTextEdit;
class QPushButton;
class QGraphicsView;
class QGraphicsScene;
class QTimer;

class ReplyScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ReplyScreen(QWidget *parent = nullptr);
    ~ReplyScreen();
    void showTrajectoryGraph(const std::vector<Simulator::Sample>& samples,
                            const std::vector<std::size_t>& activeTargetIndices,
                            std::size_t selectedTargetIndex,
                            std::size_t visibleSampleCount = 0);

private slots:
    void on_closeButton_clicked();

private:
    QGraphicsView *trajectoryGraphView;
    QGraphicsScene *trajectoryScene;
    QPushButton *closeButton;
    QTimer *replayTimer;

    std::vector<Simulator::Sample> replaySamples;
    std::vector<std::size_t> replayTargetIndices;
    std::size_t replaySelectedTargetIndex;
    std::size_t replayVisibleSampleCount;
    std::size_t replayCurrentFrame;

    void drawScaleBar(const QRectF& sceneRect, double sceneScale);
    void renderTrajectoryFrame(std::size_t visibleSampleCount);

private slots:
    void onReplayTick();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // REPLAYSCREEN_H
