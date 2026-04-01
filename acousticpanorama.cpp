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
    headerLabel->setObjectName("acousticHeaderLabel");
    headerLabel->setMinimumHeight(30);
    headerLabel->setMaximumHeight(30);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    
    // Graphics View for displaying graphs
    graphicsView = new QGraphicsView(this);
    graphicsView->setObjectName("acousticGraphicsView");
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 200, 200);
    graphicsView->setScene(scene);
    
    mainLayout->addWidget(graphicsView);
}


