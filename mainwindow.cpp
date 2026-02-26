#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "eventtablewidget.h"
#include "trajectoryview.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QAction>
#include <QStackedWidget>
#include <QLabel>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , eventView(nullptr)
    , trajectoryView(nullptr)
    , viewStack(nullptr)
{
    ui->setupUi(this);
    
    // Setup view stack for content switching
    setupViewStack();
    
    // Add some sample events for demonstration
    eventView->addEvent("Sensor1", "10:30:45.123", "10:30:50.456", "10:31:00.789", "CONTACT", "Target Detected");
    eventView->addEvent("Sensor2", "10:31:00.234", "10:31:05.567", "10:31:15.890", "ENGAGEMENT", "Firing Solution Computed");
    eventView->addEvent("Sensor1", "10:31:15.345", "10:31:20.678", "10:31:30.001", "DETONATION", "Impact Detected");
    
    // Connect menu actions
    connect(ui->actionFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(ui->actionFileClose, &QAction::triggered, this, &MainWindow::onFileClose);
    connect(ui->actionFileExit, &QAction::triggered, this, &MainWindow::onFileExit);
    
    // Connect View menu actions
    connect(ui->actionViewParameter, &QAction::triggered, this, &MainWindow::onViewParameter);
    connect(ui->actionViewEvent, &QAction::triggered, this, &MainWindow::onViewEvent);
    connect(ui->actionViewTrajectory, &QAction::triggered, this, &MainWindow::onViewTrajectory);
    
    // Connect Print menu actions
    connect(ui->actionPrintParameterView, &QAction::triggered, this, &MainWindow::onPrintParameterView);
    connect(ui->actionPrintEventView, &QAction::triggered, this, &MainWindow::onPrintEventView);
    connect(ui->actionPrintTrajectoryView, &QAction::triggered, this, &MainWindow::onPrintTrajectoryView);
    
    // Connect event table row click to trajectory view
    connect(eventView, &EventTableWidget::rowClicked, this, &MainWindow::onEventRowClicked);
    
    // Set standard window size, but clamp to the available screen size
    const QSize standardSize(1200, 1600);
    QSize targetSize = standardSize;
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QSize availableSize = screen->availableGeometry().size();
        targetSize = standardSize.boundedTo(availableSize);
        setMaximumSize(availableSize);
    }
    setMinimumSize(QSize(0, 0));
    resize(targetSize);
    
    // Configure left table - resize columns to fit content
    ui->GeoInfoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->GeoInfoTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->GeoInfoTable->setAlternatingRowColors(true);
    
    // Show parameter view by default
    switchToParameterView();
}

void MainWindow::setupViewStack()
{
    // Create the stacked widget to hold the different views
    viewStack = new QStackedWidget(this);
    
    // The parameter view is the original UI created by ui->setupUi(this)
    // Add it directly to the stacked widget
    viewStack->addWidget(ui->centralwidget);    // Index 0: Parameter view
    
    // Create and add event view
    eventView = new EventTableWidget(this);
    viewStack->addWidget(eventView);            // Index 1: Event view
    
    // Create and add trajectory view
    trajectoryView = new TrajectoryView(this);
    viewStack->addWidget(trajectoryView);       // Index 2: Trajectory view
    
    // Set the stacked widget as the central widget
    setCentralWidget(viewStack);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Open File"), "", 
        tr("Data Files (*.dat *.txt);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        qDebug() << "Opening file:" << fileName;
        QMessageBox::information(this, tr("File Opened"), 
            tr("Opened: %1").arg(fileName));
    }
}

void MainWindow::onFileClose()
{
    qDebug() << "Closing file";
    QMessageBox::information(this, tr("File Closed"), 
        tr("Current file has been closed."));
}

void MainWindow::onFileExit()
{
    qDebug() << "Exiting application";
    QApplication::quit();
}

void MainWindow::onViewParameter()
{
    qDebug() << "Switching to Parameter view";
    switchToParameterView();
    QMessageBox::information(this, tr("View Changed"), 
        tr("Switched to Parameter view."));
}

void MainWindow::onViewEvent()
{
    qDebug() << "Switching to Event view";
    switchToEventView();
}

void MainWindow::onViewTrajectory()
{
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}

void MainWindow::onPrintParameterView()
{
    qDebug() << "Printing Parameter View";
    QMessageBox::information(this, tr("Print"), 
        tr("Parameter View sent to printer."));
}

void MainWindow::onPrintEventView()
{
    qDebug() << "Printing Event View";
    QMessageBox::information(this, tr("Print"), 
        tr("Event View sent to printer."));
}

void MainWindow::onPrintTrajectoryView()
{
    qDebug() << "Printing Trajectory View";
    QMessageBox::information(this, tr("Print"), 
        tr("Trajectory View sent to printer."));
}

void MainWindow::switchToParameterView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(0);
        qDebug() << "Switched to Parameter view (index 0)";
    }
}

void MainWindow::switchToEventView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(1);
        qDebug() << "Switched to Event view (index 1)";
    }
}

void MainWindow::switchToTrajectoryView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(2);
        qDebug() << "Switched to Trajectory view (index 2)";
    }
}

void MainWindow::onEventRowClicked(int row)
{
    qDebug() << "Event row clicked:" << row;
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}
