#include "trajectoryview.h"
#include "geographicalinfo.h"
#include "acousticpanorama.h"
#include "demongraph.h"
#include "trajectorytable.h"
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
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QtMath>
#include <cstdlib>
#include <QRandomGenerator>
#include <fstream>
#include <sstream>

TrajectoryView::TrajectoryView(QWidget *parent)
    : QWidget(parent)
    , trajectoryPlot(nullptr)
    , tubeSelect(nullptr)
    , tablePlot(nullptr)
    , acousticPanorama(nullptr)
    , demonGraph(nullptr)
    , geoInfo(nullptr)
    , replyButton(nullptr)
    , trajectoryScene(nullptr)
    , animationTimer(new QTimer(this))
    , currentStep(0)
{
    setupUI();
    initTrajectoryPlot();
    
    // Install event filter on the graphics view for zoom handling
    trajectoryPlot->installEventFilter(this);
    
    populateDummyData();

    connect(animationTimer, &QTimer::timeout, this, &TrajectoryView::updateAnimation);
}

void TrajectoryView::loadTrajectoryData(const QString &filePath)
{
    trajectoryData.clear();
    std::ifstream file(filePath.toStdString());
    if (!file.is_open()) {
        return;
    }

    std::string line;
    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string value;
        Simulator::Sample sample;

        std::getline(ss, sample.timestamp, ',');
        std::getline(ss, value, ',');
        sample.ownSpeedKnots = std::stod(value);
        std::getline(ss, value, ',');
        sample.ownHeadingDeg = std::stod(value);
        std::getline(ss, value, ',');
        sample.targetSpeedKnots = std::stod(value);
        std::getline(ss, value, ',');
        sample.targetHeadingDeg = std::stod(value);
        std::getline(ss, value, ',');
        sample.torpedoSpeedKnots = std::stod(value);
        std::getline(ss, value, ',');
        sample.torpedoHeadingDeg = std::stod(value);
        std::getline(ss, value, ',');
        sample.targetX = std::stod(value);
        std::getline(ss, value, ',');
        sample.targetY = std::stod(value);
        std::getline(ss, value, ',');
        sample.torpedoX = std::stod(value);
        std::getline(ss, value, ',');
        sample.torpedoY = std::stod(value);

        trajectoryData.push_back(sample);
    }

    file.close();
    populateDummyData();
    plotTrajectory(trajectoryData.size());
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
    
    replyButton = new QPushButton("Reply");
    replyButton->setObjectName("trajectoryReplyButton");
    replyButton->setMinimumHeight(30);
    replyButton->setMaximumWidth(200);
    tubeSelectLayout->addWidget(replyButton);
    
    tubeSelect = new QColumnView();
    tubeSelect->setObjectName("trajectoryTubeSelect");
    tubeSelect->setMaximumWidth(200);
    tubeSelectLayout->addWidget(tubeSelect);

    QHBoxLayout *animationButtonsLayout = new QHBoxLayout();
    QPushButton *startButton = new QPushButton("Start");
    QPushButton *stopButton = new QPushButton("Stop");
    animationButtonsLayout->addWidget(startButton);
    animationButtonsLayout->addWidget(stopButton);
    tubeSelectLayout->addLayout(animationButtonsLayout);

    connect(startButton, &QPushButton::clicked, this, &TrajectoryView::startAnimation);
    connect(stopButton, &QPushButton::clicked, this, &TrajectoryView::stopAnimation);
    
    topRightLayout->addLayout(tubeSelectLayout);
    
    // Trajectory Plot
    trajectoryPlot = new QGraphicsView();
    trajectoryPlot->setObjectName("trajectoryPlotView");
    trajectoryPlot->setFrameShape(QFrame::NoFrame);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    trajectoryPlot->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    trajectoryPlot->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    topRightLayout->addWidget(trajectoryPlot);
    
    rightLayout->addLayout(topRightLayout, 1);
    
    // Table Plot
    
    tablePlot = new TrajectoryTable();
    // Add a container widget for tablePlot to provide margin
    QWidget *tablePlotContainer = new QWidget();
    tablePlotContainer->setObjectName("trajectoryTableContainer");
    QVBoxLayout *tablePlotContainerLayout = new QVBoxLayout(tablePlotContainer);
    tablePlotContainerLayout->setContentsMargins(10, 10, 10, 10); // Add space around the table
    tablePlotContainerLayout->addWidget(tablePlot, 0, Qt::AlignTop);
    rightLayout->addWidget(tablePlotContainer, 1);
    
    // Add to main layout
    mainLayout->addWidget(leftPanel);

    mainLayout->addWidget(rightPanel, 1);

    connect(replyButton, &QPushButton::clicked, this, &TrajectoryView::on_replyButton_clicked);
}

void TrajectoryView::startAnimation()
{
    currentStep = 0;
    animationTimer->start(100); // Update every 100ms
}

void TrajectoryView::stopAnimation()
{
    animationTimer->stop();
}

void TrajectoryView::updateAnimation()
{
    if (currentStep >= trajectoryData.size()) {
        animationTimer->stop();
        return;
    }

    plotTrajectory(currentStep);
    currentStep++;
}


void TrajectoryView::on_replyButton_clicked()
{
    ReplyScreen *replyScreen = new ReplyScreen();
    replyScreen->show();
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
    trajectoryScene->setBackgroundBrush(QBrush(QColor(0, 0, 0)));
    
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
    trajectoryPlot->setDragMode(QGraphicsView::ScrollHandDrag);
    if (trajectoryPlot->dragMode() == QGraphicsView::ScrollHandDrag) {
        trajectoryPlot->setCursor(Qt::OpenHandCursor);
    } else {
        trajectoryPlot->setCursor(Qt::ArrowCursor);
    }
}

void TrajectoryView::plotTrajectory(size_t currentStep)
{
    trajectoryScene->clear();
    initTrajectoryPlot();

    if (trajectoryData.size() < 2) {
        return;
    }

    QPen targetPen(QColor(128, 128, 128)); // gray
    targetPen.setWidth(2);

    QPen torpedoPen(QColor(0, 51, 102)); // darkBlue
    torpedoPen.setWidth(2);

    for (size_t i = 1; i < currentStep; ++i) {
        if (i >= trajectoryData.size()) break;
        const auto& prev = trajectoryData[i - 1];
        const auto& curr = trajectoryData[i];

        // Draw target trajectory
        trajectoryScene->addLine(prev.targetX, prev.targetY, curr.targetX, curr.targetY, targetPen);

        // Draw torpedo trajectory
        trajectoryScene->addLine(prev.torpedoX, prev.torpedoY, curr.torpedoX, curr.torpedoY, torpedoPen);
    }
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
