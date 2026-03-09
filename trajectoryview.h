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

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();

private:
    // Main layout widgets
    QGraphicsView *trajectoryPlot;
    QColumnView *tubeSelect;
    QColumnView *tablePlot;
    
    // Left side widgets
    QLabel *acousticLabel;
    QGraphicsView *acousticGraphWidget;
    QLabel *demonLabel;
    QGraphicsView *demonGraphWidget;
    QLabel *geoHeaderLabel;
    QTableWidget *geoTable;
    QPushButton *replayButton;
    
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
