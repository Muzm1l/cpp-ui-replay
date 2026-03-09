#include "acousticpanorama.h"
#include <QPainter>

AcousticPanorama::AcousticPanorama(QWidget *parent)
    : QWidget(parent)
    , mainLayout(nullptr)
    , headerLabel(nullptr)
    , graphicsView(nullptr)
    , scene(nullptr)
{
    setupUI();
}

AcousticPanorama::~AcousticPanorama()
{
    // Qt handles cleanup of child widgets automatically
}

void AcousticPanorama::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header Label
    headerLabel = new QLabel("Acoustic Panorama");
    headerLabel->setMinimumHeight(30);
    headerLabel->setMaximumHeight(30);
    headerLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px; border: 1px solid gray;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    
    // Graphics View for displaying graphs
    graphicsView = new QGraphicsView(this);
    graphicsView->setStyleSheet("background-color: rgb(50, 50, 50); border: 1px solid gray;");
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 200, 200);
    graphicsView->setScene(scene);
    
    mainLayout->addWidget(graphicsView);
}


