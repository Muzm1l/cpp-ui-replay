#include "trajectoryview.h"
#include "ui_trajectoryview.h"
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

TrajectoryView::TrajectoryView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TrajectoryView)
{
    ui->setupUi(this);
    initTrajectoryPlot();
    
    // Install event filter on the graphics view for zoom handling
    ui->TrajectoryPlot->installEventFilter(this);
    
    populateDummyData();
}

TrajectoryView::~TrajectoryView()
{
    delete ui;
}

void TrajectoryView::populateDummyData()
{
    // Configure Geo Info Table - resize columns to fit content
    ui->geoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->geoTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->geoTable->setAlternatingRowColors(true);
    
    // Populate Geo Info Table with dummy data
    ui->geoTable->setItem(0, 1, new QTableWidgetItem("10:31:45.123"));
    ui->geoTable->setItem(1, 1, new QTableWidgetItem("125.5°"));
    ui->geoTable->setItem(2, 1, new QTableWidgetItem("5500 m"));
    ui->geoTable->setItem(3, 1, new QTableWidgetItem("089.2°"));
    ui->geoTable->setItem(4, 1, new QTableWidgetItem("8200 m"));
    ui->geoTable->setItem(5, 1, new QTableWidgetItem("156.8°"));
    ui->geoTable->setItem(6, 1, new QTableWidgetItem("2700 m"));
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
    
    // Draw coordinate system with margins for axes
    QPen gridPen(white, 0.5);
    gridPen.setStyle(Qt::DashLine);
    for (int i = 100; i <= 1000; i += 100) {
        trajectoryScene->addLine(i, 50, i, 750, gridPen);
    }
    for (int i = 50; i <= 750; i += 100) {
        trajectoryScene->addLine(100, i, 970, i, gridPen);
    }
    
    // Draw axes - positioned at edges with ItemIgnoresTransformations flag
    QPen axisPen(white, 2);
    QGraphicsLineItem *xAxis = trajectoryScene->addLine(100, 750, 970, 750, axisPen);
    QGraphicsLineItem *yAxis = trajectoryScene->addLine(100, 50, 100, 750, axisPen);
    
    // Make axes ignore view transformations (stay fixed)
    xAxis->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    yAxis->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    
    axisLines.append(xAxis);
    axisLines.append(yAxis);
    
    // Add axis scale markers and labels
    QFont scaleFont("Arial", 6);
    QFont labelFont("Arial", 7);
    
    // X-axis labels (Range)
    for (int i = 100; i <= 970; i += 150) {
        QGraphicsLineItem *marker = trajectoryScene->addLine(i, 745, i, 755, axisPen);
        marker->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        axisLines.append(marker);
        
        QGraphicsTextItem *scaleLabel = trajectoryScene->addText(QString::number((i - 100) * 50));
        scaleLabel->setDefaultTextColor(white);
        scaleLabel->setFont(scaleFont);
        scaleLabel->setPos(i - 15, 760);
        scaleLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        axisLabels.append(scaleLabel);
    }
    
    // Y-axis labels (Bearing)
    for (int i = 100; i <= 750; i += 150) {
        QGraphicsLineItem *marker = trajectoryScene->addLine(95, i, 105, i, axisPen);
        marker->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        axisLines.append(marker);
        
        QGraphicsTextItem *bearingLabel = trajectoryScene->addText(QString::number((750 - i) / 10));
        bearingLabel->setDefaultTextColor(white);
        bearingLabel->setFont(scaleFont);
        bearingLabel->setPos(50, i - 8);
        bearingLabel->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        axisLabels.append(bearingLabel);
    }
    
    // Add axis labels
    QGraphicsTextItem *labelX = trajectoryScene->addText("Range (m)");
    labelX->setDefaultTextColor(white);
    labelX->setFont(labelFont);
    labelX->setPos(880, 775);
    labelX->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    axisLabels.append(labelX);
    
    QGraphicsTextItem *labelY = trajectoryScene->addText("Bearing (°)");
    labelY->setDefaultTextColor(white);
    labelY->setFont(labelFont);
    labelY->setPos(10, 40);
    labelY->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    axisLabels.append(labelY);
    
    // ============ MAIN VISUALIZATION ============
    
    // OwnShip position (center reference) - cyan circle with crosshair
    QGraphicsEllipseItem *ownship = trajectoryScene->addEllipse(350, 380, 12, 12, QPen(cyan, 1.5), QBrush(cyan));
    trajectoryScene->addLine(344, 386, 336, 386, QPen(cyan, 1.5));  // Left crosshair
    trajectoryScene->addLine(362, 386, 370, 386, QPen(cyan, 1.5));  // Right crosshair
    trajectoryScene->addLine(356, 374, 356, 366, QPen(cyan, 1.5));  // Top crosshair
    trajectoryScene->addLine(356, 398, 356, 406, QPen(cyan, 1.5));  // Bottom crosshair
    
    QGraphicsTextItem *ownshipLabel = trajectoryScene->addText("OwnShip\n[0m]");
    ownshipLabel->setDefaultTextColor(cyan);
    ownshipLabel->setFont(QFont("Arial", 7));
    ownshipLabel->setPos(375, 382);
    
    // Torpedo track - dark blue line with waypoints
    QPen torpedoPen(darkBlue, 2);
    trajectoryScene->addLine(365, 400, 600, 180, torpedoPen);
    
    // Torpedo waypoints
    trajectoryScene->addEllipse(420, 340, 4, 4, QPen(yellow, 0.5), QBrush(yellow));
    trajectoryScene->addEllipse(480, 290, 4, 4, QPen(yellow, 0.5), QBrush(yellow));
    trajectoryScene->addEllipse(540, 240, 4, 4, QPen(yellow, 0.5), QBrush(yellow));
    
    // Torpedo tip with marker
    QGraphicsEllipseItem *torpedoTip = trajectoryScene->addEllipse(595, 175, 12, 12, QPen(darkBlue, 1.5), QBrush(darkBlue));
    QGraphicsTextItem *torpedoLabel = trajectoryScene->addText("T [5200m]");
    torpedoLabel->setDefaultTextColor(darkBlue);
    torpedoLabel->setFont(QFont("Arial", 7));
    torpedoLabel->setPos(610, 177);
    
    // Target ship - gray circle with marker
    QGraphicsEllipseItem *target = trajectoryScene->addEllipse(745, 270, 12, 12, QPen(gray, 1.5), QBrush(gray));
    trajectoryScene->addLine(750, 266, 750, 258, QPen(gray, 1));  // Heading indicator
    
    QGraphicsTextItem *targetLabel = trajectoryScene->addText("TGT [8200m]");
    targetLabel->setDefaultTextColor(gray);
    targetLabel->setFont(QFont("Arial", 7));
    targetLabel->setPos(762, 272);
    
    // Target trajectory - gray line with dashes
    QPen targetTrajPen(gray, 1.5);
    targetTrajPen.setStyle(Qt::DashLine);
    trajectoryScene->addLine(760, 290, 850, 430, targetTrajPen);
    
    // Target predicted position
    QGraphicsEllipseItem *targetPredicted = trajectoryScene->addEllipse(845, 425, 8, 8, QPen(gray, 1), QBrush());
    targetPredicted->setOpacity(0.5);
    QGraphicsTextItem *targetPredLabel = trajectoryScene->addText("Pred");
    targetPredLabel->setDefaultTextColor(gray);
    targetPredLabel->setFont(QFont("Arial", 6));
    targetPredLabel->setPos(858, 422);
    
    // Selected aiming line - red (fire solution)
    QPen selectedPen(red, 2.5);
    trajectoryScene->addLine(365, 400, 755, 280, selectedPen);
    
    // Fire solution intersection point
    QGraphicsEllipseItem *firePoint = trajectoryScene->addEllipse(750, 275, 8, 8, QPen(red, 1.5), QBrush());
    firePoint->setOpacity(0.7);
    
    // Distance and bearing annotations
    QGraphicsTextItem *distAnnot = trajectoryScene->addText("CPA: 2700m");
    distAnnot->setDefaultTextColor(white);
    distAnnot->setFont(QFont("Arial", 7));
    distAnnot->setPos(480, 310);
    
    QGraphicsTextItem *timeAnnot = trajectoryScene->addText("TOF: 45 sec");
    timeAnnot->setDefaultTextColor(white);
    timeAnnot->setFont(QFont("Arial", 7));
    timeAnnot->setPos(480, 325);
    
    // Set scene to graphics view
    ui->TrajectoryPlot->setScene(trajectoryScene);
    ui->TrajectoryPlot->setRenderHint(QPainter::Antialiasing);
    ui->TrajectoryPlot->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->TrajectoryPlot->fitInView(0, 0, 1050, 850, Qt::KeepAspectRatio);
}

bool TrajectoryView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->TrajectoryPlot && event->type() == QEvent::Wheel) {
        QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
        
        const double zoomFactor = 1.15;
        
        if (wheelEvent->angleDelta().y() > 0) {
            // Zoom in
            zoomLevel *= zoomFactor;
            ui->TrajectoryPlot->scale(zoomFactor, zoomFactor);
        } else {
            // Zoom out
            zoomLevel /= zoomFactor;
            ui->TrajectoryPlot->scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        }
        
        // Keep axes visible - ensure minimum zoom
        if (zoomLevel < 0.8) {
            zoomLevel = 0.8;
            ui->TrajectoryPlot->fitInView(0, 0, 1050, 850, Qt::KeepAspectRatio);
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
