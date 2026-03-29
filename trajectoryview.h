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
#include "geographicalinfo.h"
#include "trajectorytable.h"
#include "acousticpanorama.h"
#include "demongraph.h"
#include "replyscreen.h"
#include "simulator.h"
#include <QTimer>
#include <vector>

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();
    void loadTrajectoryData(const QString& filePath);

public slots:
    void startAnimation();
    void stopAnimation();

private slots:
    void on_replyButton_clicked();
    void updateAnimation();

private:
    // Main layout widgets
    QGraphicsView *trajectoryPlot;
    QColumnView *tubeSelect;
    TrajectoryTable *tablePlot;
    
    // Left side widgets
    AcousticPanorama *acousticPanorama;
    DemonGraph *demonGraph;
    GeographicalInfo *geoInfo;
    QPushButton *replyButton;
    
    // Graphics scene
    double zoomLevel = 1.0;
    QGraphicsScene *trajectoryScene = nullptr;
    
    // Axes items to keep fixed
    QList<QGraphicsLineItem*> axisLines;
    QList<QGraphicsTextItem*> axisLabels;
    
    std::vector<Simulator::Sample> trajectoryData;

    QTimer *animationTimer;
    size_t currentStep;
    
    void setupUI();
    void initTrajectoryPlot();
    void populateDummyData();
    void updateAxesPosition();
    void plotTrajectory(size_t currentStep);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // TRAJECTORYVIEW_H
