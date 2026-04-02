#include "replyscreen.h"
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QResizeEvent>
#include <QPushButton>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QtMath>
#include <algorithm>
#include <cmath>

ReplyScreen::ReplyScreen(QWidget *parent)
    : QWidget(parent)
    , trajectoryGraphView(nullptr)
    , trajectoryScene(nullptr)
{
    setObjectName("replyScreen");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    trajectoryGraphView = new QGraphicsView(this);
    trajectoryGraphView->setObjectName("replyTrajectoryGraph");
    trajectoryGraphView->setStyleSheet("background: transparent; border: 0.5px solid whitesmoke;");
    trajectoryGraphView->setFrameShape(QFrame::NoFrame);
    trajectoryGraphView->setRenderHint(QPainter::Antialiasing);
    trajectoryGraphView->setRenderHint(QPainter::SmoothPixmapTransform);
    trajectoryGraphView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    trajectoryGraphView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    trajectoryScene = new QGraphicsScene(this);
    trajectoryScene->setBackgroundBrush(Qt::NoBrush);
    trajectoryGraphView->setScene(trajectoryScene);
    layout->addWidget(trajectoryGraphView, 1);

    closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("dangerCloseButton");
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    connect(closeButton, &QPushButton::clicked, this, &ReplyScreen::on_closeButton_clicked);
}

ReplyScreen::~ReplyScreen()
{
}

void ReplyScreen::showTrajectoryGraph(const std::vector<Simulator::Sample>& samples,
                                      const std::vector<std::size_t>& activeTargetIndices,
                                      std::size_t selectedTargetIndex,
                                      std::size_t visibleSampleCount)
{
    if (!trajectoryScene || !trajectoryGraphView) {
        return;
    }

    trajectoryScene->clear();

    if (samples.empty()) {
        return;
    }

    const int viewWidth = trajectoryGraphView->viewport()->width() > 0 ? trajectoryGraphView->viewport()->width() : 1050;
    const int viewHeight = trajectoryGraphView->viewport()->height() > 0 ? trajectoryGraphView->viewport()->height() : 850;
    const QRectF sceneRect(0, 0, viewWidth, viewHeight);
    trajectoryScene->setSceneRect(sceneRect);

    const QPointF sceneCenter(sceneRect.width() / 2.0, sceneRect.height() / 2.0);
    const double ownshipOriginX = samples.front().ownX;
    const double ownshipOriginY = samples.front().ownY;

    std::vector<std::size_t> targetsToDraw = activeTargetIndices;
    if (targetsToDraw.empty() && !samples.front().targets.empty()) {
        for (std::size_t i = 0; i < samples.front().targets.size(); ++i) {
            targetsToDraw.push_back(i);
        }
    }

    double maxAbsCoord = 1.0;
    for (const auto& sample : samples) {
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.ownX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.ownY - ownshipOriginY));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.targetX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.targetY - ownshipOriginY));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.torpedoX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.torpedoY - ownshipOriginY));
        for (std::size_t targetIndex : targetsToDraw) {
            if (targetIndex >= sample.targets.size()) {
                continue;
            }
            const auto& target = sample.targets[targetIndex];
            maxAbsCoord = std::max(maxAbsCoord, std::abs(target.x - ownshipOriginX));
            maxAbsCoord = std::max(maxAbsCoord, std::abs(target.y - ownshipOriginY));
        }
    }

    const double plotHalfWidth = (sceneRect.width() / 2.0) - 50.0;
    const double plotHalfHeight = (sceneRect.height() / 2.0) - 50.0;
    const double sceneScale = std::min(plotHalfWidth, plotHalfHeight) / maxAbsCoord;

    const auto toScenePoint = [sceneCenter, sceneScale](double relX, double relY) -> QPointF {
        return QPointF(sceneCenter.x() + (relX * sceneScale), sceneCenter.y() - (relY * sceneScale));
    };

    const double legendRightMargin = 20.0;
    const double legendTopStart = 10.0;
    const double legendSpacing = 20.0;

    auto* targetLabel = trajectoryScene->addText("Target Track");
    targetLabel->setDefaultTextColor(QColor(220, 80, 80));
    targetLabel->setPos(sceneRect.width() - targetLabel->boundingRect().width() - legendRightMargin, legendTopStart);

    auto* otherTargetsLabel = trajectoryScene->addText("Other Targets");
    otherTargetsLabel->setDefaultTextColor(QColor(170, 170, 170));
    otherTargetsLabel->setPos(sceneRect.width() - otherTargetsLabel->boundingRect().width() - legendRightMargin,
                              legendTopStart + legendSpacing);

    auto* torpedoLabel = trajectoryScene->addText("Torpedo Track");
    torpedoLabel->setDefaultTextColor(QColor(0, 120, 255));
    torpedoLabel->setPos(sceneRect.width() - torpedoLabel->boundingRect().width() - legendRightMargin,
                         legendTopStart + (2.0 * legendSpacing));

    auto* ownshipLabel = trajectoryScene->addText("Ownship Track");
    ownshipLabel->setDefaultTextColor(QColor(0, 255, 255));
    ownshipLabel->setPos(sceneRect.width() - ownshipLabel->boundingRect().width() - legendRightMargin,
                         legendTopStart + (3.0 * legendSpacing));

    const std::size_t drawSampleCount = visibleSampleCount == 0
                                            ? samples.size()
                                            : std::min(visibleSampleCount, samples.size());

    for (std::size_t i = 1; i < drawSampleCount; ++i) {
        const auto& previous = samples[i - 1];
        const auto& current = samples[i];

        const QPointF previousOwnship = toScenePoint(previous.ownX - ownshipOriginX, previous.ownY - ownshipOriginY);
        const QPointF currentOwnship = toScenePoint(current.ownX - ownshipOriginX, current.ownY - ownshipOriginY);
        QPen ownshipPen(QColor(0, 255, 255));
        ownshipPen.setWidth(2);
        trajectoryScene->addLine(QLineF(previousOwnship, currentOwnship), ownshipPen);

        for (std::size_t targetIndex : targetsToDraw) {
            if (targetIndex >= previous.targets.size() || targetIndex >= current.targets.size()) {
                continue;
            }
            const QPointF previousTarget = toScenePoint(previous.targets[targetIndex].x - ownshipOriginX,
                                                        previous.targets[targetIndex].y - ownshipOriginY);
            const QPointF currentTarget = toScenePoint(current.targets[targetIndex].x - ownshipOriginX,
                                                       current.targets[targetIndex].y - ownshipOriginY);

            QPen targetPen(targetIndex == selectedTargetIndex ? QColor(220, 80, 80) : QColor(170, 170, 170));
            targetPen.setWidth(targetIndex == selectedTargetIndex ? 3 : 2);
            trajectoryScene->addLine(QLineF(previousTarget, currentTarget), targetPen);
        }

        if (previous.torpedoSpeedKnots > 0.0 || current.torpedoSpeedKnots > 0.0) {
            const QPointF previousTorpedo = toScenePoint(previous.torpedoX - ownshipOriginX, previous.torpedoY - ownshipOriginY);
            const QPointF currentTorpedo = toScenePoint(current.torpedoX - ownshipOriginX, current.torpedoY - ownshipOriginY);
            QPen torpedoPen(QColor(0, 120, 255));
            torpedoPen.setWidth(2);
            trajectoryScene->addLine(QLineF(previousTorpedo, currentTorpedo), torpedoPen);
        }
    }

    drawScaleBar(sceneRect, sceneScale);
    trajectoryGraphView->fitInView(sceneRect, Qt::KeepAspectRatio);
}

void ReplyScreen::drawScaleBar(const QRectF& sceneRect, double sceneScale)
{
    if (!trajectoryScene) {
        return;
    }

    const double scaleBarLength = 1000.0 * sceneScale;
    const double scaleBarX = 60.0;
    const double scaleBarY = sceneRect.height() - 60.0;

    QPen scalePen(QColor(255, 255, 200));
    scalePen.setWidth(2);

    trajectoryScene->addLine(scaleBarX, scaleBarY, scaleBarX + scaleBarLength, scaleBarY, scalePen);
    trajectoryScene->addLine(scaleBarX, scaleBarY - 5, scaleBarX, scaleBarY + 5, scalePen);
    trajectoryScene->addLine(scaleBarX + scaleBarLength, scaleBarY - 5, scaleBarX + scaleBarLength, scaleBarY + 5, scalePen);

    auto* scaleLabel = trajectoryScene->addText("1000m");
    scaleLabel->setDefaultTextColor(QColor(255, 255, 200));
    scaleLabel->setPos(scaleBarX + scaleBarLength / 2.0 - 20.0, scaleBarY + 10.0);
}

void ReplyScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (trajectoryGraphView && trajectoryScene) {
        trajectoryGraphView->fitInView(trajectoryScene->sceneRect(), Qt::KeepAspectRatio);
    }
}

void ReplyScreen::on_closeButton_clicked()
{
    close();
}
