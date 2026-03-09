#include "trajectoryview.h"
#include "geographicalinfo.h"
#include "acousticpanorama.h"
#include "demongraph.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QWheelEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QtMath>
#include <cstdlib>
#include <QRandomGenerator>

TrajectoryView::TrajectoryView(QWidget *parent)
    : QWidget(parent)
    , trajectoryPlot(nullptr)
    , tubeSelect(nullptr)
    , tablePlot(nullptr)
    , acousticPanorama(nullptr)
    , demonGraph(nullptr)
    , geoInfo(nullptr)
    , replayButton(nullptr)
    , trajectoryScene(nullptr)
{
    setupUI();
    initTrajectoryPlot();
    
    // Install event filter on the graphics view for zoom handling
    trajectoryPlot->installEventFilter(this);
    
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
    
    replayButton = new QPushButton("Replay");
    replayButton->setMinimumHeight(30);
    replayButton->setMaximumWidth(200);
    replayButton->setStyleSheet(
        "QPushButton {\n"
        "  background-color: rgb(85, 87, 83);\n"
        "  color: white;\n"
        "  font-weight: bold;\n"
        "  border: 1px solid gray;\n"
        "}\n"
        "QPushButton:hover {\n"
        "  background-color: rgb(100, 100, 100);\n"
        "}\n"
        "QPushButton:pressed {\n"
        "  background-color: rgb(70, 70, 70);\n"
        "}"
    );
    tubeSelectLayout->addWidget(replayButton);
    
    tubeSelect = new QColumnView();
    tubeSelect->setMaximumWidth(200);
    tubeSelect->setStyleSheet("border: 1px solid gray;");
    tubeSelectLayout->addWidget(tubeSelect);
    
    topRightLayout->addLayout(tubeSelectLayout);
    
    // Trajectory Plot
    trajectoryPlot = new QGraphicsView();
    trajectoryPlot->setStyleSheet("background-color: rgb(50, 50, 50); border: 1px solid gray;");
    trajectoryPlot->setFrameShape(QFrame::NoFrame);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    topRightLayout->addWidget(trajectoryPlot);
    
    rightLayout->addLayout(topRightLayout, 1);
    
    // Table Plot
    tablePlot = new QColumnView();
    tablePlot->setFrameShadow(QFrame::Sunken);
    tablePlot->setStyleSheet("border: 1px solid gray;");
    rightLayout->addWidget(tablePlot, 1);
    
    // Add to main layout
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
}

void TrajectoryView::populateDummyData()
{
    geoInfo->clearData();
    // acousticPanorama is now a simple empty widget
    demonGraph->clearData();
}

void TrajectoryView::initTrajectoryPlot()
{
    // Create graphics scene with extra margins for axes
    trajectoryScene = new QGraphicsScene(this);
    trajectoryScene->setSceneRect(0, 0, 1050, 850);
    trajectoryScene->setBackgroundBrush(QBrush(QColor(50, 50, 50)));
    
    // Define colors
    QColor darkBlue(0, 51, 102);      // Torpedo track
    QColor red(255, 0, 0);            // Selected
    QColor cyan(0, 255, 255);         // OwnShip
    QColor gray(128, 128, 128);       // Target ship
    QColor white(255, 255, 255);      // Grid/axes
    QColor yellow(255, 255, 0);       // Waypoints
    
    // Set scene to graphics view
    trajectoryPlot->setScene(trajectoryScene);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    trajectoryPlot->setRenderHint(QPainter::SmoothPixmapTransform);
    trajectoryPlot->fitInView(0, 0, 1050, 850, Qt::KeepAspectRatio);
}

bool TrajectoryView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == trajectoryPlot && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        
        const double zoomFactor = 1.15;
        
        if (wheelEvent->angleDelta().y() > 0) {
            // Zoom in
            zoomLevel *= zoomFactor;
            trajectoryPlot->scale(zoomFactor, zoomFactor);
        } else {
            // Zoom out
            zoomLevel /= zoomFactor;
            trajectoryPlot->scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        }
        
        // Keep axes visible - ensure minimum zoom
        if (zoomLevel < 0.8) {
            zoomLevel = 0.8;
            trajectoryPlot->fitInView(0, 0, 1050, 850, Qt::KeepAspectRatio);
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
