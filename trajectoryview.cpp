#include "trajectoryview.h"
#include "geographicalinfo.h"
#include "acousticpanorama.h"
#include "demongraph.h"
#include "trajectorytable.h"
#include "simulator.h"
#include "replayscreen.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPushButton>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QtMath>
#include <algorithm>
#include <cstdlib>
#include <QRandomGenerator>
#include <QVector>
#include <cmath>

TrajectoryView::TrajectoryView(QWidget *parent)
    : QWidget(parent)
    , trajectoryPlot(nullptr)
    , tubeSelect(nullptr)
    , tubeSelectModel(nullptr)
    , tablePlot(nullptr)
    , acousticPanorama(nullptr)
    , demonGraph(nullptr)
    , geoInfo(nullptr)
    , replyButton(nullptr)
    , replyScreen(nullptr)
    , trajectoryScene(nullptr)
    , animationTimer(new QTimer(this))
{
    setupUI();
    initTrajectoryPlot();
    
    // Install event filter on the graphics view for zoom handling
    trajectoryPlot->installEventFilter(this);
    animationTimer->setInterval(25);
    connect(animationTimer, &QTimer::timeout, this, &TrajectoryView::drawNextAnimationFrame);
    
    populateDummyData();
}

TrajectoryView::~TrajectoryView()
{
    // Qt handles cleanup of child widgets automatically
}

void TrajectoryView::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Left panel
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    leftPanel->setFixedWidth(400);
    
    // Acoustic Panorama Widget
    acousticPanorama = new AcousticPanorama();
    leftLayout->addWidget(acousticPanorama);
    
    // DEMON Widget
    demonGraph = new DemonGraph();
    leftLayout->addWidget(demonGraph);
    
    // Geographical Info Widget
    geoInfo = new GeographicalInfo();
    geoInfo->setFixedWidthMode(true, 400);
    leftLayout->addWidget(geoInfo);
    
    // Vertical spacer
    leftLayout->addStretch();
    
    // Right panel
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);
    
    // Top right layout with tube select and trajectory plot
    QHBoxLayout *topRightLayout = new QHBoxLayout();
    topRightLayout->setSpacing(5);
    
    // Tube Select Layout
    QVBoxLayout *tubeSelectLayout = new QVBoxLayout();
    tubeSelectLayout->setSpacing(5);
    
    replyButton = new QPushButton("Replay");
    replyButton->setMinimumHeight(30);
    replyButton->setMaximumWidth(200);
    replyButton->setStyleSheet(
        "QPushButton {\n"
        "  background-color: rgb(52, 101, 164);\n"
        "  color: white;\n"
        "  font-weight: bold;\n"
        "  border: 1px solid whitesmoke;\n"
        "}\n"
        "QPushButton:hover {\n"
        "  background-color: rgb(66, 122, 185);\n"
        "}\n"
        "QPushButton:pressed {\n"
        "  background-color: rgb(40, 84, 142);\n"
        "}"
    );
    tubeSelectLayout->addWidget(replyButton);
    connect(replyButton, &QPushButton::clicked, this, &TrajectoryView::openReplyScreen);
    
    tubeSelect = new QColumnView();
    tubeSelect->setObjectName("trajectoryTubeSelect");
    tubeSelect->setMaximumWidth(200);
    tubeSelect->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tubeSelect->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tubeSelect->setStyleSheet("border: 0.5px solid whitesmoke;");

    tubeSelectModel = new QStringListModel(this);
    tubeSelect->setModel(tubeSelectModel);
    connect(tubeSelect->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                if (updatingTubeSelection || !current.isValid()) {
                    return;
                }
                const int row = current.row();
                if (row < 0 || static_cast<std::size_t>(row) >= availableTargetIndices.size()) {
                    return;
                }
                selectedTargetIndex = availableTargetIndices[static_cast<std::size_t>(row)];
                targetSelectionConfirmed = true;
                emit targetSectorChanged(selectedTargetIsForwardSector());
                if (simulatorDataReady) {
                    replaySimulation();
                } else {
                    clearGraph();
                }
            });

    tubeSelectLayout->addWidget(tubeSelect);
    
    topRightLayout->addLayout(tubeSelectLayout);
    
    // Trajectory Plot
    trajectoryPlot = new QGraphicsView();
    trajectoryPlot->setStyleSheet("background: transparent; border: 0.5px solid whitesmoke;");
    trajectoryPlot->setFrameShape(QFrame::NoFrame);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    trajectoryPlot->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    trajectoryPlot->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    topRightLayout->addWidget(trajectoryPlot);
    
    rightLayout->addLayout(topRightLayout, 1);
    
    // Table Plot
    
    tablePlot = new TrajectoryTable();
    tablePlot->setStyleSheet("border: 0.5px solid whitesmoke;");
    // Add a container widget for tablePlot to provide margin
    QWidget *tablePlotContainer = new QWidget();
    QVBoxLayout *tablePlotContainerLayout = new QVBoxLayout(tablePlotContainer);
    tablePlotContainerLayout->setContentsMargins(0, 0, 0, 0);
    tablePlotContainerLayout->setSpacing(0);
    tablePlotContainerLayout->addWidget(tablePlot, 0, Qt::AlignTop);
    tablePlotContainer->setStyleSheet("border: 0.5px solid Whitesmoke;");
    rightLayout->addWidget(tablePlotContainer, 1);
    
    // Add to main layout
    mainLayout->addWidget(leftPanel);

    mainLayout->addWidget(rightPanel, 1);

    refreshTubeTargetList();
}

void TrajectoryView::populateDummyData()
{
    geoInfo->clearData();
    demonGraph->clearData();
    clearGraph();
}

void TrajectoryView::initTrajectoryPlot()
{
    // Create graphics scene - size will be determined dynamically
    trajectoryScene = new QGraphicsScene(this);
    trajectoryScene->setBackgroundBrush(Qt::NoBrush);
    
    // Set scene to graphics view
    trajectoryPlot->setScene(trajectoryScene);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    trajectoryPlot->setRenderHint(QPainter::SmoothPixmapTransform);
}

void TrajectoryView::drawSimulationPrototype()
{
    if (!trajectoryScene || !trajectoryPlot) {
        return;
    }

    if (!simulatorDataReady) {
        clearGraph();
        return;
    }

    animatedSamples = Simulator::generateScenarioSamples(180, 1, 0, targetCount, selectedTargetIndex, selectedTubeIndex);
    if (animatedSamples.empty()) {
        return;
    }

    animationTimer->stop();
    animationIndex = 0;
    hasPreviousTargetPoint = false;
    hasPreviousTorpedoPoint = false;
    hasPreviousOwnshipPoint = false;
    previousTargetPoints.clear();
    hasPreviousTargetPoints.clear();
    
    // Calculate dynamic scene rect based on available space
    const int viewWidth = trajectoryPlot->viewport()->width() > 0 ? trajectoryPlot->viewport()->width() : 1050;
    const int viewHeight = trajectoryPlot->viewport()->height() > 0 ? trajectoryPlot->viewport()->height() : 850;
    dynamicSceneRect = QRectF(0, 0, viewWidth, viewHeight);
    trajectoryScene->setSceneRect(dynamicSceneRect);
    
    setupStaticPlotScaffold();

    sceneCenter = QPointF(dynamicSceneRect.width() / 2.0, dynamicSceneRect.height() / 2.0);
    const double ownshipOriginX = animatedSamples.front().ownX;
    const double ownshipOriginY = animatedSamples.front().ownY;

    std::vector<std::size_t> activeTargetIndices = availableTargetIndices;
    if (activeTargetIndices.empty()) {
        for (std::size_t i = 0; i < targetCount; ++i) {
            activeTargetIndices.push_back(i);
        }
    }

    double maxAbsCoord = 1.0;
    for (const auto& sample : animatedSamples) {
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.ownX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.ownY - ownshipOriginY));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.targetX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.targetY - ownshipOriginY));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.torpedoX - ownshipOriginX));
        maxAbsCoord = std::max(maxAbsCoord, std::abs(sample.torpedoY - ownshipOriginY));
        for (std::size_t targetIndex : activeTargetIndices) {
            if (targetIndex >= sample.targets.size()) {
                continue;
            }
            const auto& target = sample.targets[targetIndex];
            maxAbsCoord = std::max(maxAbsCoord, std::abs(target.x - ownshipOriginX));
            maxAbsCoord = std::max(maxAbsCoord, std::abs(target.y - ownshipOriginY));
        }
    }

    const double plotHalfWidth = (dynamicSceneRect.width() / 2.0) - 40.0;
    const double plotHalfHeight = (dynamicSceneRect.height() / 2.0) - 40.0;
    sceneScale = std::min(plotHalfWidth, plotHalfHeight) / maxAbsCoord;

    // Add 1000m scale indicator after sceneScale is calculated
    drawScaleBar(dynamicSceneRect);

    zoomLevel = 1.0;
    trajectoryPlot->resetTransform();
    trajectoryPlot->fitInView(dynamicSceneRect, Qt::KeepAspectRatio);

    // TrajectoryView shows the complete stamped trajectory without animation.
    while (animationIndex < animatedSamples.size()) {
        drawNextAnimationFrame();
    }
}

void TrajectoryView::setupStaticPlotScaffold()
{
    trajectoryScene->clear();
    trajectoryScene->setBackgroundBrush(Qt::NoBrush);

    const QPointF center(dynamicSceneRect.width() / 2.0, dynamicSceneRect.height() / 2.0);

    const double legendRightMargin = 20.0;
    const double legendTopStart = 10.0;
    const double legendSpacing = 20.0;

    auto* targetLabel = trajectoryScene->addText("Target Track");
    targetLabel->setDefaultTextColor(QColor(220, 80, 80));
    targetLabel->setPos(dynamicSceneRect.width() - targetLabel->boundingRect().width() - legendRightMargin,
                        legendTopStart);

    auto* otherTargetsLabel = trajectoryScene->addText("Other Targets");
    otherTargetsLabel->setDefaultTextColor(QColor(170, 170, 170));
    otherTargetsLabel->setPos(dynamicSceneRect.width() - otherTargetsLabel->boundingRect().width() - legendRightMargin,
                              legendTopStart + legendSpacing);

    auto* torpedoLabel = trajectoryScene->addText("Torpedo Track");
    torpedoLabel->setDefaultTextColor(QColor(0, 120, 255));
    torpedoLabel->setPos(dynamicSceneRect.width() - torpedoLabel->boundingRect().width() - legendRightMargin,
                         legendTopStart + (2.0 * legendSpacing));

    auto* ownshipLabel = trajectoryScene->addText("Ownship Track");
    ownshipLabel->setDefaultTextColor(QColor(0, 255, 255));
    ownshipLabel->setPos(dynamicSceneRect.width() - ownshipLabel->boundingRect().width() - legendRightMargin,
                         legendTopStart + (3.0 * legendSpacing));
}

void TrajectoryView::drawScaleBar(const QRectF& sceneRect)
{
    // Draw 1000m scale indicator on left side
    const double scaleBarLength = 1000.0 * sceneScale;
    const double scaleBarX = 60.0;
    const double scaleBarY = sceneRect.height() - 60.0;
    
    QPen scalePen(QColor(255, 255, 200));
    scalePen.setWidth(2);
    
    // Draw scale bar line
    trajectoryScene->addLine(scaleBarX, scaleBarY, scaleBarX + scaleBarLength, scaleBarY, scalePen);
    // Draw end caps
    trajectoryScene->addLine(scaleBarX, scaleBarY - 5, scaleBarX, scaleBarY + 5, scalePen);
    trajectoryScene->addLine(scaleBarX + scaleBarLength, scaleBarY - 5, scaleBarX + scaleBarLength, scaleBarY + 5, scalePen);
    
    // Add scale label
    auto* scaleLabel = trajectoryScene->addText("1000m");
    scaleLabel->setDefaultTextColor(QColor(255, 255, 200));
    scaleLabel->setPos(scaleBarX + scaleBarLength / 2.0 - 20, scaleBarY + 10);
}

void TrajectoryView::drawNextAnimationFrame()
{
    if (animationIndex >= animatedSamples.size()) {
        animationTimer->stop();
        return;
    }

    const auto toScenePoint = [this](double relX, double relY) -> QPointF {
        return QPointF(sceneCenter.x() + (relX * sceneScale), sceneCenter.y() - (relY * sceneScale));
    };

    const Simulator::Sample& sample = animatedSamples[animationIndex];
    const double ownshipOriginX = animatedSamples.front().ownX;
    const double ownshipOriginY = animatedSamples.front().ownY;

    std::vector<std::size_t> activeTargetIndices = availableTargetIndices;
    if (activeTargetIndices.empty()) {
        for (std::size_t i = 0; i < sample.targets.size(); ++i) {
            activeTargetIndices.push_back(i);
        }
    }

    if (previousTargetPoints.size() != sample.targets.size()) {
        previousTargetPoints.assign(sample.targets.size(), QPointF());
        hasPreviousTargetPoints.assign(sample.targets.size(), false);
    }

    const QPointF ownshipPoint = toScenePoint(sample.ownX - ownshipOriginX, sample.ownY - ownshipOriginY);
    if (hasPreviousOwnshipPoint) {
        QPen ownshipPen(QColor(0, 255, 255));
        ownshipPen.setWidth(2);
        trajectoryScene->addLine(QLineF(previousOwnshipPoint, ownshipPoint), ownshipPen);
    }
    previousOwnshipPoint = ownshipPoint;
    hasPreviousOwnshipPoint = true;

    for (std::size_t i : activeTargetIndices) {
        if (i >= sample.targets.size()) {
            continue;
        }

        const QPointF targetPoint = toScenePoint(sample.targets[i].x - ownshipOriginX, sample.targets[i].y - ownshipOriginY);
        if (hasPreviousTargetPoints[i]) {
            QPen targetPen(i == sample.selectedTargetIndex ? QColor(220, 80, 80) : QColor(170, 170, 170));
            targetPen.setWidth(i == sample.selectedTargetIndex ? 3 : 2);
            trajectoryScene->addLine(QLineF(previousTargetPoints[i], targetPoint), targetPen);
        }
        previousTargetPoints[i] = targetPoint;
        hasPreviousTargetPoints[i] = true;
    }

    const QPointF selectedTargetPoint = toScenePoint(sample.targetX - ownshipOriginX, sample.targetY - ownshipOriginY);
    previousTargetPoint = selectedTargetPoint;
    hasPreviousTargetPoint = true;

    if (sample.torpedoSpeedKnots > 0.0) {
        const QPointF torpedoPoint = toScenePoint(sample.torpedoX - ownshipOriginX, sample.torpedoY - ownshipOriginY);
        if (hasPreviousTorpedoPoint) {
            QPen torpedoPen(QColor(0, 120, 255));
            torpedoPen.setWidth(2);
            trajectoryScene->addLine(QLineF(previousTorpedoPoint, torpedoPoint), torpedoPen);
        }
        previousTorpedoPoint = torpedoPoint;
        hasPreviousTorpedoPoint = true;
    }

    const std::size_t stampedFrameIndex = animationIndex;
    ++animationIndex;

    updateGeoInfoForSample(sample);
    if (tablePlot) {
        tablePlot->updateStampedInfo(sample, stampedFrameIndex);
    }
}

bool TrajectoryView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == trajectoryPlot && event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            zoomFocusScenePoint = trajectoryPlot->mapToScene(mouseEvent->pos());
            hasZoomFocusPoint = true;
        }
    }

    if (obj == trajectoryPlot && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);

        const int deltaY = wheelEvent->angleDelta().y();
        if (deltaY == 0) {
            return true;
        }

        // Smooth zoom based on actual wheel delta (trackpad/mouse friendly)
        const double zoomPerDeltaStep = 1.0022;
        const double requestedScaleFactor = std::pow(zoomPerDeltaStep, static_cast<double>(deltaY));
        const double minZoomLevel = 1.0;
        const double maxZoomLevel = 40.0;
        const double requestedZoomLevel = zoomLevel * requestedScaleFactor;
        const double clampedZoomLevel = std::clamp(requestedZoomLevel, minZoomLevel, maxZoomLevel);

        if (clampedZoomLevel <= minZoomLevel && deltaY < 0) {
            zoomLevel = 1.0;
            trajectoryPlot->resetTransform();
            trajectoryPlot->fitInView(dynamicSceneRect, Qt::KeepAspectRatio);
            return true;
        }

        const double effectiveScaleFactor = clampedZoomLevel / zoomLevel;

        if (qFuzzyCompare(effectiveScaleFactor, 1.0)) {
            return true;
        }

        zoomLevel = clampedZoomLevel;

        if (hasZoomFocusPoint) {
            trajectoryPlot->setTransformationAnchor(QGraphicsView::NoAnchor);
            trajectoryPlot->scale(effectiveScaleFactor, effectiveScaleFactor);
            trajectoryPlot->centerOn(zoomFocusScenePoint);
        } else {
            trajectoryPlot->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            trajectoryPlot->scale(effectiveScaleFactor, effectiveScaleFactor);
            trajectoryPlot->setTransformationAnchor(QGraphicsView::NoAnchor);
        }
        
        return true;
    }
    
    return QWidget::eventFilter(obj, event);
}

void TrajectoryView::updateAxesPosition()
{
    // Axes positions are maintained by ItemIgnoresTransformations flag
    // This method is kept for future axis repositioning if needed
}

bool TrajectoryView::isTubeSelectionVisible() const
{
    return tubeSelect && tubeSelect->isVisible();
}

void TrajectoryView::setTubeSelectionVisible(bool visible)
{
    if (tubeSelect) {
        tubeSelect->setVisible(visible);
    }
}

void TrajectoryView::setSelectedTargetIndex(std::size_t index)
{
    const std::size_t safeTargetCount = targetCount == 0 ? 1 : targetCount;
    selectedTargetIndex = std::min(index, safeTargetCount - 1);

    if (!tubeSelectModel || tubeSelectModel->rowCount() == 0) {
        return;
    }

    int rowToSelect = -1;
    for (std::size_t i = 0; i < availableTargetIndices.size(); ++i) {
        if (availableTargetIndices[i] == selectedTargetIndex) {
            rowToSelect = static_cast<int>(i);
            break;
        }
    }

    if (rowToSelect < 0 && !availableTargetIndices.empty()) {
        selectedTargetIndex = availableTargetIndices.front();
        rowToSelect = 0;
    }

    QModelIndex targetModelIndex = tubeSelectModel->index(rowToSelect, 0);
    if (!targetModelIndex.isValid()) {
        return;
    }

    updatingTubeSelection = true;
    tubeSelect->setCurrentIndex(targetModelIndex);
    updatingTubeSelection = false;

    emit targetSectorChanged(selectedTargetIsForwardSector());

    if (!animatedSamples.empty()) {
        updateGeoInfoForSample(animatedSamples[std::min(animationIndex, animatedSamples.size() - 1)]);
    }
}

void TrajectoryView::setAvailableTargetsForTube(const std::vector<std::size_t>& targetIndices,
                                                const QStringList& targetLabels)
{
    availableTargetIndices = targetIndices;
    availableTargetLabels = targetLabels;
    refreshTubeTargetList();
    setSelectedTargetIndex(selectedTargetIndex);
}

void TrajectoryView::setSimulatorDataReady(bool ready)
{
    simulatorDataReady = ready;
    clearGraph();
}

void TrajectoryView::setTubeSelectionConfirmed(bool confirmed)
{
    tubeSelectionConfirmed = confirmed;
    if (!tubeSelectionConfirmed) {
        clearGraph();
        if (geoInfo) {
            geoInfo->clearData();
        }
    }
}

void TrajectoryView::setTargetCount(std::size_t count)
{
    targetCount = count == 0 ? 1 : count;
    if (availableTargetIndices.empty()) {
        availableTargetIndices.clear();
        const std::size_t safeTargetCount = targetCount == 0 ? 1 : targetCount;
        for (std::size_t i = 0; i < safeTargetCount; ++i) {
            availableTargetIndices.push_back(i);
        }
        availableTargetLabels.clear();
    }
    refreshTubeTargetList();
    setSelectedTargetIndex(selectedTargetIndex);
}

void TrajectoryView::setSelectedTubeIndex(std::size_t index)
{
    selectedTubeIndex = index;
}

std::size_t TrajectoryView::currentSelectedTargetIndex() const
{
    return selectedTargetIndex;
}

std::size_t TrajectoryView::currentTargetCount() const
{
    return targetCount == 0 ? 1 : targetCount;
}

bool TrajectoryView::selectedTargetIsForwardSector() const
{
    const std::size_t safeTargetCount = currentTargetCount();
    const std::vector<Simulator::Sample> samples = Simulator::generateScenarioSamples(1, 1, 0, safeTargetCount, selectedTargetIndex, selectedTubeIndex);
    if (samples.empty()) {
        return true;
    }

    const Simulator::Sample& sample = samples.front();
    if (sample.targets.empty() || selectedTargetIndex >= sample.targets.size()) {
        return true;
    }

    const auto& target = sample.targets[selectedTargetIndex];
    const double headingRad = qDegreesToRadians(sample.ownHeadingDeg);
    const double forwardX = std::sin(headingRad);
    const double forwardY = std::cos(headingRad);
    const double relX = target.x - sample.ownX;
    const double relY = target.y - sample.ownY;
    return (relX * forwardX) + (relY * forwardY) >= 0.0;
}

void TrajectoryView::replaySimulation()
{
    drawSimulationPrototype();
}

void TrajectoryView::openReplyScreen()
{
    if (!simulatorDataReady || !tubeSelectionConfirmed || !targetSelectionConfirmed) {
        return;
    }

    if (!replyScreen) {
        replyScreen = new ReplyScreen(nullptr);
    }

    std::vector<Simulator::Sample> samplesToShow = animatedSamples;
    if (samplesToShow.empty()) {
        samplesToShow = Simulator::generateScenarioSamples(180, 1, 0, targetCount, selectedTargetIndex, selectedTubeIndex);
    }

    if (samplesToShow.empty()) {
        return;
    }

    std::vector<std::size_t> targetsToShow = availableTargetIndices;
    if (targetsToShow.empty()) {
        for (std::size_t i = 0; i < targetCount; ++i) {
            targetsToShow.push_back(i);
        }
    }

    replyScreen->showFullScreen();
    replyScreen->raise();
    replyScreen->activateWindow();
    replyScreen->showTrajectoryGraph(samplesToShow,
                                     targetsToShow,
                                     selectedTargetIndex,
                                     samplesToShow.size());
}

void TrajectoryView::updateGeoInfoForSample(const Simulator::Sample& sample)
{
    if (!geoInfo) {
        return;
    }

    if (!simulatorDataReady || !tubeSelectionConfirmed || !targetSelectionConfirmed) {
        geoInfo->clearData();
        return;
    }

    const std::size_t selectedIndex = std::min(selectedTargetIndex, sample.targets.empty() ? std::size_t(0) : sample.targets.size() - 1);
    if (sample.targets.empty() || selectedIndex >= sample.targets.size()) {
        geoInfo->clearData();
        return;
    }

    const auto& target = sample.targets[selectedIndex];
    geoInfo->updateGeoDataFromPositions(static_cast<float>(animationIndex),
                                        static_cast<float>(sample.ownX), static_cast<float>(sample.ownY),
                                        static_cast<float>(target.x), static_cast<float>(target.y),
                                        static_cast<float>(sample.torpedoX), static_cast<float>(sample.torpedoY));
}

void TrajectoryView::refreshTubeTargetList()
{
    if (!tubeSelectModel) {
        return;
    }

    QStringList targetNumbers;
    if (!availableTargetLabels.isEmpty() && availableTargetLabels.size() == static_cast<int>(availableTargetIndices.size())) {
        targetNumbers = availableTargetLabels;
    } else {
        for (std::size_t i = 0; i < availableTargetIndices.size(); ++i) {
            targetNumbers << QString::number(1001 + static_cast<int>(availableTargetIndices[i]));
        }
    }
    tubeSelectModel->setStringList(targetNumbers);

    QFont targetFont = tubeSelect->font();
    targetFont.setPointSize(targetFont.pointSize() > 0 ? targetFont.pointSize() + 4 : 16);

    for (int row = 0; row < tubeSelectModel->rowCount(); ++row) {
        const QModelIndex index = tubeSelectModel->index(row, 0);
        tubeSelectModel->setData(index, targetFont, Qt::FontRole);
        tubeSelectModel->setData(index, QVariant::fromValue(Qt::AlignHCenter | Qt::AlignVCenter), Qt::TextAlignmentRole);
    }

    if (geoInfo) {
        geoInfo->clearData();
    }
}

void TrajectoryView::clearGraph()
{
    if (!trajectoryScene || !trajectoryPlot) {
        return;
    }

    animationTimer->stop();
    trajectoryScene->clear();
    trajectoryScene->setBackgroundBrush(Qt::NoBrush);

    const int viewWidth = trajectoryPlot->viewport()->width() > 0 ? trajectoryPlot->viewport()->width() : 1050;
    const int viewHeight = trajectoryPlot->viewport()->height() > 0 ? trajectoryPlot->viewport()->height() : 850;
    dynamicSceneRect = QRectF(0, 0, viewWidth, viewHeight);
    trajectoryScene->setSceneRect(dynamicSceneRect);
    zoomLevel = 1.0;
    trajectoryPlot->resetTransform();
    trajectoryPlot->fitInView(dynamicSceneRect, Qt::KeepAspectRatio);

    if (tablePlot) {
        tablePlot->clearStampedInfo();
    }
}
