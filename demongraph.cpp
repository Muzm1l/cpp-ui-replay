#include "demongraph.h"
#include <QPainter>

DemonGraph::DemonGraph(QWidget *parent)
    : QWidget(parent)
    , mainLayout(nullptr)
    , headerLabel(nullptr)
    , graphicsView(nullptr)
    , scene(nullptr)
{
    setupUI();
}

DemonGraph::~DemonGraph()
{
    // Qt handles cleanup of child widgets automatically
}

void DemonGraph::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header Label
    headerLabel = new QLabel("DEMON Analysis");
    headerLabel->setMinimumHeight(30);
    headerLabel->setMaximumHeight(30);
    headerLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px; border: 0.5px solid whitesmoke;");
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    
    // Graphics View for displaying graphs
    graphicsView = new QGraphicsView(this);
    graphicsView->setStyleSheet("background-color: rgb(50, 50, 50); border: 0.5px solid whitesmoke;");
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 200, 200);
    graphicsView->setScene(scene);
    
    mainLayout->addWidget(graphicsView);
}

void DemonGraph::clearData()
{
    // Placeholder for compatibility
    scene->clear();
}


