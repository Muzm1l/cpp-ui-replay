#ifndef REPLYSCREEN_H
#define REPLYSCREEN_H

#include <QWidget>
#include <vector>

#include "simulator.h"

class QTextEdit;
class QPushButton;
class QGraphicsView;
class QGraphicsScene;

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

    void drawScaleBar(const QRectF& sceneRect, double sceneScale);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // REPLYSCREEN_H
