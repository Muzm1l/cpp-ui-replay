#include "home.h"
#include "parser.h"
#include "parserdialog.h"
#include "eventview.h"
#include "parametrview.h"
#include "trajectoryview.h"
#include "geographicalinfo.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QAction>
#include <QStackedWidget>
#include <QGraphicsScene>
#include <QBrush>

Home::Home(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , viewStack(nullptr)
    , menuBar(nullptr)
    , parameterView(nullptr)
    , eventView(nullptr)
    , trajectoryView(nullptr)
    , homeWidget(nullptr)
    , trajectoryPlot(nullptr)
    , tubeSelect(nullptr)
    , tablePlot(nullptr)
    , geoInfo(nullptr)
    , trajectoryScene(nullptr)
{
    // Setup the window
    setWindowTitle("Home");
    // Set the main window background color to black
    this->setStyleSheet("background-color: black;");
    // Create main UI
    setupUI();
    
    // Setup view stack for content switching
    setupViewStack();
    
    // Setup home widget with trajectory view layout
    setupHomeWidget();
    
    // Add some sample events for demonstration
    eventView->addEvent("Sensor1", "10:30:45.123", "10:30:50.456", "10:31:00.789", "CONTACT", "Target Detected");
    eventView->addEvent("Sensor2", "10:31:00.234", "10:31:05.567", "10:31:15.890", "ENGAGEMENT", "Firing Solution Computed");
    eventView->addEvent("Sensor1", "10:31:15.345", "10:31:20.678", "10:31:30.001", "DETONATION", "Impact Detected");
    
    // Connect menu actions
    connect(actionFileOpen, &QAction::triggered, this, &Home::onFileOpen);
    connect(actionFileClose, &QAction::triggered, this, &Home::onFileClose);
    connect(actionFileParser, &QAction::triggered, this, &Home::onFileParser);
    connect(actionFileExit, &QAction::triggered, qApp, &QApplication::quit);
    
    // Connect View menu actions
    connect(actionViewParameter, &QAction::triggered, this, &Home::onViewParameter);
    connect(actionViewEvent, &QAction::triggered, this, &Home::onViewEvent);
    connect(actionViewTrajectory, &QAction::triggered, this, &Home::onViewTrajectory);
    
    // Connect Print menu actions
    connect(actionPrintParameterView, &QAction::triggered, this, &Home::onPrintParameterView);
    connect(actionPrintEventView, &QAction::triggered, this, &Home::onPrintEventView);
    connect(actionPrintTrajectoryView, &QAction::triggered, this, &Home::onPrintTrajectoryView);
    
    // Connect event table row click to trajectory view
    connect(eventView, &EventView::rowClicked, this, &Home::onEventRowClicked);
    
    // Set full screen mode
    showFullScreen();
}

Home::~Home()
{
    // Qt handles cleanup of child widgets automatically
}

void Home::setupUI()
{
    // Setup menu bar
    setupMenuBar();
    
    // Setup status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
}

void Home::setupMenuBar()
{
    menuBar = new QMenuBar(this);

    menuBar->setStyleSheet("QMenuBar { background-color: gray; color: white;  } QMenuBar::item { background-color: gray; color: white; } QMenuBar::item:selected { background-color: #a0a0a0; color: white; }");
    menuBar->setFixedHeight(32);
    setMenuBar(menuBar);
    
    // File Menu
    menuFile = menuBar->addMenu("File");
    
    actionFileOpen = menuFile->addAction("Open");
    actionFileOpen->setShortcut(Qt::CTRL | Qt::Key_O);
    
    actionFileClose = menuFile->addAction("Close");
    actionFileClose->setShortcut(Qt::CTRL | Qt::Key_W);
    
    menuFile->addSeparator();
    
    actionFileExit = menuFile->addAction("Exit");
    actionFileExit->setShortcut(Qt::CTRL | Qt::Key_Q);
    
    // View Menu
    menuView = menuBar->addMenu("View");
    
    actionViewParameter = menuView->addAction("Parameter");
    actionViewEvent = menuView->addAction("Event");
    actionViewTrajectory = menuView->addAction("Trajectory");
    
    // Print Menu
    menuPrint = menuBar->addMenu("Print");
    
    actionPrintParameterView = menuPrint->addAction("Parameter View");
    actionPrintEventView = menuPrint->addAction("Event View");
    actionPrintTrajectoryView = menuPrint->addAction("Trajectory View");
    
    // Parser Menu
    menuParser = menuBar->addMenu("Parser");
    
    actionFileParser = menuParser->addAction("Parser");
    actionFileParser->setShortcut(Qt::CTRL | Qt::Key_P);
}

void Home::setupViewStack()
{
    // Create the stacked widget to hold the different views
    viewStack = new QStackedWidget(this);
    
    // Create home widget (will be set up separately)
    homeWidget = new QWidget(this);
    viewStack->addWidget(homeWidget);           // Index 0: Home view
    
    // Create and add parameter view
    parameterView = new ParametrView(this);
    viewStack->addWidget(parameterView);        // Index 1: Parameter view
    
    // Create and add event view
    eventView = new EventView(this);
    viewStack->addWidget(eventView);            // Index 2: Event view
    
    // Create and add trajectory view
    trajectoryView = new TrajectoryView(this);
    viewStack->addWidget(trajectoryView);       // Index 3: Trajectory view
    
    // Set the stacked widget as the central widget
    setCentralWidget(viewStack);
    
    // Show home view by default
    viewStack->setCurrentIndex(0);
}


void Home::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Open File"), "", 
        tr("Data Files (*.dat *.txt);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        qDebug() << "Opening file:" << fileName;

        DatParser parser;
        if (!parser.parseFile(fileName.toStdString())) {
            QMessageBox::critical(this, tr("Parser"), tr("Failed to parse selected file."));
            return;
        }

        currentDatFilePath = fileName;
    }
}

void Home::onFileClose()
{
    qDebug() << "Closing file";
    QMessageBox::information(this, tr("File Closed"), 
        tr("Current file has been closed."));
}

void Home::onFileParser()
{
    if (!currentDatFilePath.trimmed().isEmpty()) {
        ParserDialog::show(this, currentDatFilePath, true);
        return;
    }

    ParserDialog::show(this);
}

void Home::onViewParameter()
{
    qDebug() << "Switching to Parameter view";
    switchToParameterView();
}

void Home::onViewEvent()
{
    qDebug() << "Switching to Event view";
    switchToEventView();
}

void Home::onViewTrajectory()
{
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}

void Home::onPrintParameterView()
{
    qDebug() << "Printing Parameter View";
    QMessageBox::information(this, tr("Print"), 
        tr("Parameter View sent to printer."));
}

void Home::onPrintEventView()
{
    qDebug() << "Printing Event View";
    QMessageBox::information(this, tr("Print"), 
        tr("Event View sent to printer."));
}

void Home::onPrintTrajectoryView()
{
    qDebug() << "Printing Trajectory View";
    QMessageBox::information(this, tr("Print"), 
        tr("Trajectory View sent to printer."));
}

void Home::switchToParameterView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(1);
        qDebug() << "Switched to Parameter view (index 1)";
    }
}

void Home::switchToEventView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(2);
        qDebug() << "Switched to Event view (index 2)";
    }
}

void Home::switchToTrajectoryView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(3);
        qDebug() << "Switched to Trajectory view (index 3)";
    }
}

void Home::onEventRowClicked(int row)
{
    qDebug() << "Event row clicked:" << row;
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}

void Home::setupHomeWidget()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(homeWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Left panel (only Geographical Info, no acoustic/demon)
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    leftPanel->setFixedWidth(400);
    
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
    
    tubeSelect = new QColumnView();
    tubeSelect->setMaximumWidth(150);
    tubeSelect->setStyleSheet("border: 1px solid whitesmoke;");
    tubeSelectLayout->addWidget(tubeSelect);
    
    topRightLayout->addLayout(tubeSelectLayout);
    
    // Trajectory Plot
    trajectoryPlot = new QGraphicsView();
    trajectoryPlot->setStyleSheet("background-color: none ; border: 1px solid whitesmoke;");
    trajectoryPlot->setFrameShape(QFrame::Box);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
    topRightLayout->addWidget(trajectoryPlot);
    
    rightLayout->addLayout(topRightLayout, 1);
    
    // Table Plot
    tablePlot = new QColumnView();
    tablePlot->setFrameShadow(QFrame::Sunken);
    tablePlot->setStyleSheet("border: 1px solid whitesmoke;");
    rightLayout->addWidget(tablePlot, 1);
    
    // Add to main layout
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
    
    // Initialize trajectory plot
    initTrajectoryPlot();
    
    // Populate with dummy data
    populateDummyData();
}

void Home::initTrajectoryPlot()
{
    // Create empty graphics scene
    trajectoryScene = new QGraphicsScene(this);
    trajectoryScene->setSceneRect(0, 0, 800, 600);
    trajectoryScene->setBackgroundBrush(QBrush(QColor(50, 50, 50)));  // Gray background
    
    // Set scene to graphics view
    trajectoryPlot->setScene(trajectoryScene);
    trajectoryPlot->setRenderHint(QPainter::Antialiasing);
}

void Home::populateDummyData()
{
    if (geoInfo) {
        geoInfo->clearData();
    }
}
