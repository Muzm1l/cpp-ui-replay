#ifndef TRAJECTORYVIEW_H
#define TRAJECTORYVIEW_H

#include <QWidget>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QColumnView>
#include <QStringListModel>
#include "geographicalinfo.h"
#include "trajectorytable.h"
#include "acousticpanorama.h"
#include "demongraph.h"
#include "simulator.h"
#include "replayscreen.h"
#include <QTimer>
#include <QPointF>
#include <vector>

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();
    bool isTubeSelectionVisible() const;
    void setTubeSelectionVisible(bool visible);
    void setSimulatorDataReady(bool ready);
    void setTubeSelectionConfirmed(bool confirmed);
    void setSelectedTargetIndex(std::size_t index);
    void setAvailableTargetsForTube(const std::vector<std::size_t>& targetIndices,
                                    const QStringList& targetLabels = QStringList());
    void setTargetCount(std::size_t count);
    void setSelectedTubeIndex(std::size_t index);
    std::size_t currentSelectedTargetIndex() const;
    std::size_t currentTargetCount() const;
    void replaySimulation();
    void clearGraph();
    
signals:
     void targetSectorChanged(bool forwardSector);

private slots:
    void drawSimulationPrototype();
    void drawNextAnimationFrame();
    void openReplyScreen();

private:
    // Main layout widgets
    QGraphicsView *trajectoryPlot;
    QColumnView *tubeSelect;
    QStringListModel *tubeSelectModel;
    TrajectoryTable *tablePlot;
    
    // Left side widgets
    AcousticPanorama *acousticPanorama;
    DemonGraph *demonGraph;
    GeographicalInfo *geoInfo;
    QPushButton *replyButton;
    ReplyScreen *replyScreen;
    
    // Graphics scene and animation
    double zoomLevel = 1.0;
    QGraphicsScene *trajectoryScene = nullptr;
    QTimer *animationTimer;
    QRectF dynamicSceneRect;
    
    // Animation state
    std::vector<Simulator::Sample> animatedSamples;
    size_t animationIndex = 0;
    QPointF sceneCenter;
    double sceneScale = 1.0;
    QPointF previousTargetPoint;
    QPointF previousTorpedoPoint;
    QPointF previousOwnshipPoint;
    std::vector<QPointF> previousTargetPoints;
    std::vector<bool> hasPreviousTargetPoints;
    bool hasPreviousTargetPoint = false;
    bool hasPreviousTorpedoPoint = false;
    bool hasPreviousOwnshipPoint = false;
    bool updatingTubeSelection = false;
    bool simulatorDataReady = false;
    bool tubeSelectionConfirmed = false;
    bool targetSelectionConfirmed = false;
    std::size_t selectedTubeIndex = 0;
    std::size_t selectedTargetIndex = 0;
    std::size_t targetCount = 4;
    std::vector<std::size_t> availableTargetIndices;
    QStringList availableTargetLabels;
    QPointF zoomFocusScenePoint;
    bool hasZoomFocusPoint = false;
     bool selectedTargetIsForwardSector() const;
    
    void setupUI();
    void initTrajectoryPlot();
    void populateDummyData();
    void setupStaticPlotScaffold();
    void drawScaleBar(const QRectF& sceneRect);
    void updateAxesPosition();
    void refreshTubeTargetList();
    void updateGeoInfoForSample(const Simulator::Sample& sample);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // TRAJECTORYVIEW_H
