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

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();

private slots:
    void on_replyButton_clicked();

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
    
    void setupUI();
    void initTrajectoryPlot();
    void populateDummyData();
    void updateAxesPosition();
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // TRAJECTORYVIEW_H
