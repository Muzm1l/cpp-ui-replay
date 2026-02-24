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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , eventView(nullptr)
    , currentView(nullptr)
{
    ui->setupUi(this);
    
    // Create the event view (initially hidden)
    eventView = new EventTableWidget(this);
    eventView->hide();
    
    // Create the trajectory view (initially hidden)
    trajectoryView = new TrajectoryView(this);
    trajectoryView->hide();
    
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
    
    // Set size constraints
    setMinimumSize(1200, 800);   // Minimum window size
    setMaximumSize(2560, 1440);  // Maximum window size
    
    // Maximize window to fit screen
    showMaximized();
    
    // Set vertical stretch factors for the right side to make bottom view half the height
    // Find the right vertical layout
    QHBoxLayout* mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout && mainLayout->count() >= 2) {
        QLayoutItem* rightItem = mainLayout->itemAt(1);
        if (rightItem) {
            QVBoxLayout* rightLayout = qobject_cast<QVBoxLayout*>(rightItem->layout());
            if (rightLayout && rightLayout->count() >= 2) {
                qDebug() << "Setting stretch factors for right side - layout found with" << rightLayout->count() << "items";
                rightLayout->setStretch(0, 1);  // Top layout gets stretch factor 1
                rightLayout->setStretch(1, 1);  // Bottom columnView_3 gets stretch factor 1 (equal = half each)
                qDebug() << "Stretch factor for item 0:" << rightLayout->stretch(0);
                qDebug() << "Stretch factor for item 1:" << rightLayout->stretch(1);
            }
        }
    } else {
        qDebug() << "Main layout structure not as expected";
    }
    
    // Configure left table - resize columns to fit content
    ui->tableWidget_left->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_left->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget_left->setAlternatingRowColors(true);

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
    hideEventView();
    hideTrajectoryView();
    // Show other views - they should be visible by default
    ui->columnView_2->show();
    ui->columnView->show();
    ui->columnView_3->show();
    ui->tableWidget_left->show();
    QMessageBox::information(this, tr("View Changed"), 
        tr("Switched to Parameter view."));
}

void MainWindow::onViewEvent()
{
    qDebug() << "Switching to Event view";
    showEventView();
}

void MainWindow::onViewTrajectory()
{
    qDebug() << "Switching to Trajectory view";
    hideEventView();
    showTrajectoryView();
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

void MainWindow::showEventView()
{
    // Hide all other views
    ui->tableWidget_left->hide();
    ui->columnView_2->hide();
    ui->columnView->hide();
    ui->columnView_3->hide();
    
    // Get the main layout
    QHBoxLayout* mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        // Clear the main layout
        while (mainLayout->count() > 0) {
            mainLayout->removeItem(mainLayout->itemAt(0));
        }
        
        // Add event view to fill the entire central widget
        mainLayout->addWidget(eventView, 1);
        eventView->show();
        qDebug() << "Event view shown in full screen";
    }
}

void MainWindow::hideEventView()
{
    // Remove event view from layout
    QHBoxLayout* mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        // Clear the main layout
        while (mainLayout->count() > 0) {
            mainLayout->removeItem(mainLayout->itemAt(0));
        }
        
        // Create left layout
        QVBoxLayout* leftLayout = new QVBoxLayout();
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(0);
        
        // Add header label
        QLabel* headerLabel = new QLabel("Geographical Information");
        headerLabel->setMinimumSize(0, 30);
        headerLabel->setMaximumSize(400, 30);
        headerLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
        leftLayout->addWidget(headerLabel);
        
        // Add left table
        leftLayout->addWidget(ui->tableWidget_left);
        
        // Add spacers
        leftLayout->addSpacing(40);
        leftLayout->addSpacing(40);
        leftLayout->addSpacing(40);
        
        // Create right layout
        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(5);
        
        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(5);
        topLayout->addWidget(ui->columnView_2);
        topLayout->addWidget(ui->columnView, 1);
        
        rightLayout->addLayout(topLayout);
        rightLayout->addWidget(ui->columnView_3, 1);
        rightLayout->setStretch(0, 1);
        rightLayout->setStretch(1, 1);
        
        // Add layouts to main layout
        mainLayout->addLayout(leftLayout);
        mainLayout->addLayout(rightLayout, 1);
        
        // Show the views
        ui->tableWidget_left->show();
        ui->columnView_2->show();
        ui->columnView->show();
        ui->columnView_3->show();
        
        eventView->hide();
        qDebug() << "Event view hidden, restored original layout";
    }
}

void MainWindow::showTrajectoryView()
{
    // Hide all other views
    ui->tableWidget_left->hide();
    ui->columnView_2->hide();
    ui->columnView->hide();
    ui->columnView_3->hide();
    
    // Get the main layout
    QHBoxLayout* mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        // Clear the main layout
        while (mainLayout->count() > 0) {
            mainLayout->removeItem(mainLayout->itemAt(0));
        }
        
        // Add trajectory view to fill the entire central widget
        mainLayout->addWidget(trajectoryView, 1);
        trajectoryView->show();
        qDebug() << "Trajectory view shown in full screen";
    }
}

void MainWindow::hideTrajectoryView()
{
    // Remove trajectory view from layout
    QHBoxLayout* mainLayout = qobject_cast<QHBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        // Clear the main layout
        while (mainLayout->count() > 0) {
            mainLayout->removeItem(mainLayout->itemAt(0));
        }
        
        // Create left layout
        QVBoxLayout* leftLayout = new QVBoxLayout();
        leftLayout->setContentsMargins(0, 0, 0, 0);
        leftLayout->setSpacing(0);
        
        // Add header label
        QLabel* headerLabel = new QLabel("Geographical Information");
        headerLabel->setMinimumSize(0, 30);
        headerLabel->setMaximumSize(400, 30);
        headerLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
        leftLayout->addWidget(headerLabel);
        
        // Add left table
        leftLayout->addWidget(ui->tableWidget_left);
        
        // Add spacers
        leftLayout->addSpacing(40);
        leftLayout->addSpacing(40);
        leftLayout->addSpacing(40);
        
        // Create right layout
        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(5);
        
        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->setSpacing(5);
        topLayout->addWidget(ui->columnView_2);
        topLayout->addWidget(ui->columnView, 1);
        
        rightLayout->addLayout(topLayout);
        rightLayout->addWidget(ui->columnView_3, 1);
        rightLayout->setStretch(0, 1);
        rightLayout->setStretch(1, 1);
        
        // Add layouts to main layout
        mainLayout->addLayout(leftLayout);
        mainLayout->addLayout(rightLayout, 1);
        
        // Show the views
        ui->tableWidget_left->show();
        ui->columnView_2->show();
        ui->columnView->show();
        ui->columnView_3->show();
        
        trajectoryView->hide();
        qDebug() << "Trajectory view hidden, restored original layout";
    }
}

