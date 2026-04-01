#include "parametrview.h"
#include <QHeaderView>

ParametrView::ParametrView(QWidget *parent)
    : QWidget(parent), parameterTable(nullptr), layout(nullptr), leftColumnLayout(nullptr), rightLayout(nullptr), acousticPanorama(nullptr), demonGraph(nullptr),
      sliderAT(nullptr), sliderAD(nullptr), sliderAY(nullptr), sliderAX(nullptr), sliderSRT(nullptr), topRightGraphicsView(nullptr), topRightScene(nullptr), rightGraphicsView(nullptr), rightScene(nullptr)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Left panel - matching TrajectoryView sizing
    QWidget *leftPanel = new QWidget();
    leftColumnLayout = new QVBoxLayout(leftPanel);
    leftColumnLayout->setContentsMargins(0, 0, 0, 0);
    leftColumnLayout->setSpacing(5);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    leftPanel->setFixedWidth(400);
    
    // Acoustic Panorama Widget
    acousticPanorama = new AcousticPanorama();
    leftColumnLayout->addWidget(acousticPanorama);

    // DEMON Widget
    demonGraph = new DemonGraph();
    leftColumnLayout->addWidget(demonGraph);

    // Geographical Info Widget
    geographicalInfo = new GeographicalInfo();
    leftColumnLayout->addWidget(geographicalInfo);

    // Parameter Info Tree (box tree view)
    parameterInfoTree = new ParameterInfoTree();
    leftColumnLayout->addWidget(parameterInfoTree);
    // // Vertical spacer
    // leftColumnLayout->addStretch();
    
    // Right panel
    QWidget *rightPanel = new QWidget();
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);
    
    setupRightLayout();
    
    // Add panels to main layout
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
}

ParametrView::~ParametrView()
{
}

void ParametrView::setupTable()
{
    parameterTable->setColumnCount(2);
    parameterTable->setHorizontalHeaderLabels(QStringList() << "Parameter Name" << "Value");

    parameterTable->verticalHeader()->setVisible(false);
    parameterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    parameterTable->horizontalHeader()->setStretchLastSection(true);

    parameterTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    parameterTable->setSelectionMode(QAbstractItemView::SingleSelection);
    parameterTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    parameterTable->setAlternatingRowColors(true);

    parameterTable->setMinimumSize(500, 400);
}

void ParametrView::setupRightLayout()
{
    // Top section - split into 2 parts horizontally
    QWidget *topSectionWidget = new QWidget();
    topSectionWidget->setFixedHeight(450);
    QHBoxLayout *topSectionLayout = new QHBoxLayout(topSectionWidget);
    topSectionLayout->setContentsMargins(0, 0, 0, 0);
    topSectionLayout->setSpacing(5);
    
    // Left part with 4 sliders (AT, AD, AY, AX)
    QWidget *topLeftWidget = new QWidget();
    topLeftWidget->setObjectName("parameterTopLeftWidget");
    topLeftWidget->setFixedWidth(400);
    QHBoxLayout *topLeftLayout = new QHBoxLayout(topLeftWidget);
    topLeftLayout->setContentsMargins(10, 10, 10, 10);
    topLeftLayout->setSpacing(20);
    
    sliderAT = new QSlider(Qt::Vertical, this);
    sliderAT->setRange(0, 100);
    sliderAT->setMinimumHeight(300);
    QVBoxLayout *atLayout = new QVBoxLayout();
    atLayout->addWidget(sliderAT);
    QLabel *atLabel = new QLabel("AT");
    atLabel->setAlignment(Qt::AlignCenter);
    atLayout->addWidget(atLabel);
    topLeftLayout->addLayout(atLayout);
    
    sliderAD = new QSlider(Qt::Vertical, this);
    sliderAD->setRange(0, 100);
    sliderAD->setMinimumHeight(300);
    QVBoxLayout *adLayout = new QVBoxLayout();
    adLayout->addWidget(sliderAD);
    QLabel *adLabel = new QLabel("AD");
    adLabel->setAlignment(Qt::AlignCenter);
    adLayout->addWidget(adLabel);
    topLeftLayout->addLayout(adLayout);
    
    sliderAY = new QSlider(Qt::Vertical, this);
    sliderAY->setRange(0, 100);
    sliderAY->setMinimumHeight(300);
    QVBoxLayout *ayLayout = new QVBoxLayout();
    ayLayout->addWidget(sliderAY);
    QLabel *ayLabel = new QLabel("AY");
    ayLabel->setAlignment(Qt::AlignCenter);
    ayLayout->addWidget(ayLabel);
    topLeftLayout->addLayout(ayLayout);
    
    sliderAX = new QSlider(Qt::Vertical, this);
    sliderAX->setRange(0, 100);
    sliderAX->setMinimumHeight(300);
    QVBoxLayout *axLayout = new QVBoxLayout();
    axLayout->addWidget(sliderAX);
    QLabel *axLabel = new QLabel("AX");
    axLabel->setAlignment(Qt::AlignCenter);
    axLayout->addWidget(axLabel);
    topLeftLayout->addLayout(axLayout);
    
    topLeftLayout->addStretch();
    topSectionLayout->addWidget(topLeftWidget);
    
    // Right part with graphics view
    QWidget *topRightWidget = new QWidget();
    topRightWidget->setObjectName("parameterTopRightWidget");
    QVBoxLayout *topRightLayout = new QVBoxLayout(topRightWidget);
    topRightLayout->setContentsMargins(0, 0, 0, 0);
    
    topRightScene = new QGraphicsScene(this);
    topRightGraphicsView = new QGraphicsView(topRightScene, this);
    topRightGraphicsView->setObjectName("parameterTopRightGraphicsView");
    topRightGraphicsView->setFrameShape(QFrame::NoFrame);
    topRightLayout->addWidget(topRightGraphicsView);
    
    topSectionLayout->addWidget(topRightWidget, 1);
    
    rightLayout->addWidget(topSectionWidget);
    
    // Middle section with SRT slider
    QWidget *srtWidget = new QWidget();
    srtWidget->setObjectName("parameterSrtWidget");
    srtWidget->setMinimumHeight(80);
    srtWidget->setMaximumHeight(80);
    srtWidget->setFixedWidth(1100); // Set fixed width for middle section
    QHBoxLayout *srtSliderLayout = new QHBoxLayout(srtWidget);
    srtSliderLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *srtLabel = new QLabel("SRT");
    srtLabel->setAlignment(Qt::AlignCenter);
    srtSliderLayout->addWidget(srtLabel);

    sliderSRT = new QSlider(Qt::Horizontal, this);
    sliderSRT->setRange(0, 100);
    srtSliderLayout->addWidget(sliderSRT);
    
    rightLayout->addWidget(srtWidget, 0, Qt::AlignRight);
    
    // Bottom section with graphics view
    rightScene = new QGraphicsScene(this);
    rightGraphicsView = new QGraphicsView(rightScene, this);
    rightGraphicsView->setObjectName("parameterRightGraphicsView");
    rightGraphicsView->setMinimumHeight(300);
    rightLayout->addWidget(rightGraphicsView, 2);
}
