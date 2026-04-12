#ifndef PARAMETRVIEW_H
#define PARAMETRVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QSizePolicy>
#include <QTime>
#include "acousticpanorama.h"
#include "demongraph.h"
#include "geographicalinfo.h"
#include "parameterinfotree.h"
#include "scaleview.h"

class ParametrView : public QWidget
{
    Q_OBJECT

public:
    explicit ParametrView(QWidget *parent = nullptr);
    ~ParametrView();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QTableWidget *parameterTable;
    QVBoxLayout *layout;
    QVBoxLayout *leftColumnLayout;
    QVBoxLayout *rightLayout;
    AcousticPanorama *acousticPanorama;
    DemonGraph *demonGraph;
    GeographicalInfo *geographicalInfo;
    ParameterInfoTree *parameterInfoTree;

    // Right layout widgets
    QSlider *sliderAT;
    QSlider *sliderAD;
    QSlider *sliderAY;
    QSlider *sliderAX;
    ScaleView *scaleView;
    QGraphicsView *topRightGraphicsView;
    QGraphicsScene *topRightScene;
    QGraphicsView *rightGraphicsView;
    QGraphicsScene *rightScene;
    QGraphicsLineItem *topCursorLine;
    QGraphicsLineItem *bottomCursorLine;
    QGraphicsRectItem *topZoomRect;
    QGraphicsRectItem *bottomZoomRect;

    QTime engagementStartTime;
    QTime engagementEndTime;
    QTime currentCursorTime;
    QTime currentZoomStartTime;
    QTime currentZoomEndTime;
    bool hasCursorTime;
    bool hasZoomRange;

    void setupTable();
    void setupRightLayout();
    void ensureSceneRectMatchesView(QGraphicsView *view, QGraphicsScene *scene);
    double timeRatio(const QTime &time) const;
    void syncScaleToGraphics();
};

#endif // PARAMETRVIEW_H
