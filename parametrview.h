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
#include <QSizePolicy>
#include "acousticpanorama.h"
#include "demongraph.h"

class ParametrView : public QWidget
{
    Q_OBJECT

public:
    explicit ParametrView(QWidget *parent = nullptr);
    ~ParametrView();

private:
    QTableWidget *parameterTable;
    QVBoxLayout *layout;
    QVBoxLayout *leftColumnLayout;
    QVBoxLayout *rightLayout;
    AcousticPanorama *acousticPanorama;
    DemonGraph *demonGraph;
    
    // Right layout widgets
    QSlider *sliderAT;
    QSlider *sliderAD;
    QSlider *sliderAY;
    QSlider *sliderAX;
    QSlider *sliderSRT;
    QGraphicsView *topRightGraphicsView;
    QGraphicsScene *topRightScene;
    QGraphicsView *rightGraphicsView;
    QGraphicsScene *rightScene;

    void setupTable();
    void setupRightLayout();
};

#endif // PARAMETRVIEW_H
