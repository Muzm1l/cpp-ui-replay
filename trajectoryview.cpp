#include "trajectoryview.h"
#include <QHeaderView>
#include <QLabel>
#include <QFrame>

TrajectoryView::TrajectoryView(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    populateDummyData();
}

TrajectoryView::~TrajectoryView()
{
}

void TrajectoryView::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // LEFT PANEL
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);
    
    // Acoustic Panorama Graph with title
    QLabel *acousticLabel = new QLabel("Acoustic Panoroma");
    acousticLabel->setMinimumSize(0, 30);
    acousticLabel->setMaximumSize(400, 30);
    acousticLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
    acousticLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(acousticLabel);
    
    QWidget *acousticGraph = new QWidget();
    acousticGraph->setStyleSheet("background-color: rgb(50, 50, 50); border: 1px solid rgb(100, 100, 100);");
    leftLayout->addWidget(acousticGraph, 1);
    
    // Demon Graph with title
    QLabel *demonLabel = new QLabel("Demon");
    demonLabel->setMinimumSize(0, 30);
    demonLabel->setMaximumSize(400, 30);
    demonLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
    demonLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(demonLabel);
    
    QWidget *demonGraph = new QWidget();
    demonGraph->setStyleSheet("background-color: rgb(50, 50, 50); border: 1px solid rgb(100, 100, 100);");
    leftLayout->addWidget(demonGraph, 1);
    
    // Geographical information table header
    QLabel *geoHeaderLabel = new QLabel("Geographical information");
    geoHeaderLabel->setMinimumSize(0, 30);
    geoHeaderLabel->setMaximumSize(400, 30);
    geoHeaderLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
    geoHeaderLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(geoHeaderLabel);
    
    // Geographical information table
    QTableWidget *geoTable = new QTableWidget();
    geoTable->setColumnCount(2);
    geoTable->setMaximumWidth(400);
    geoTable->setRowCount(7);
    geoTable->verticalHeader()->setVisible(false);
    geoTable->horizontalHeader()->setVisible(false);
    geoTable->setStyleSheet(
        "QTableWidget { \n"
        "  gridline-color: rgb(186, 189, 182);\n"
        "  background-color: rgb(255, 255, 255);\n"
        "}\n"
        "QTableWidget::item {\n"
        "  padding: 5px;\n"
        "  background-color: rgb(255, 255, 255);\n"
        "  color: rgb(0, 0, 0);\n"
        "}"
    );
    
    // Add rows matching default page
    geoTable->setItem(0, 0, new QTableWidgetItem("Current Selected Time"));
    geoTable->setItem(1, 0, new QTableWidgetItem("Ownship / Torpedo Bearing"));
    geoTable->setItem(2, 0, new QTableWidgetItem("Ownship / Torpedo Range"));
    geoTable->setItem(3, 0, new QTableWidgetItem("Ownship / Target Bearing"));
    geoTable->setItem(4, 0, new QTableWidgetItem("Ownship / Target Range"));
    geoTable->setItem(5, 0, new QTableWidgetItem("Torpedo / Target Bearing"));
    geoTable->setItem(6, 0, new QTableWidgetItem("Torpedo / Target Range"));
    
    geoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    geoTable->verticalHeader()->setDefaultSectionSize(23);
    geoTable->setAlternatingRowColors(true);
    geoTable->setFrameShape(QFrame::NoFrame);
    geoTable->setFrameShadow(QFrame::Raised);
    geoTable->setLineWidth(0);
    leftLayout->addWidget(geoTable);
    
    mainLayout->addLayout(leftLayout);
    
    // RIGHT PANEL
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(5);
    
    // TOP RIGHT - Tube list and Trajectory plot
    QHBoxLayout *topRightLayout = new QHBoxLayout();
    topRightLayout->setSpacing(5);
    
    // Tube list
    tubeList = new QListWidget();
    tubeList->setMaximumWidth(150);
    tubeList->setStyleSheet(
        "QListWidget { \n"
        "  border: 1px solid rgb(100, 100, 100);\n"
        "  background-color: rgb(50, 50, 50);\n"
        "}\n"
        "QListWidget::item {\n"
        "  padding: 5px;\n"
        "  border-bottom: 1px solid rgb(100, 100, 100);\n"
        "  color: rgb(200, 200, 200);\n"
        "}"
    );
    topRightLayout->addWidget(tubeList);
    
    // Trajectory plot
    trajectoryPlot = new QWidget();
    trajectoryPlot->setStyleSheet("background-color: rgb(50, 50, 50); border: 1px solid rgb(100, 100, 100);");
    topRightLayout->addWidget(trajectoryPlot, 1);
    
    rightLayout->addLayout(topRightLayout);
    
    // BOTTOM RIGHT - Trajectory data table
    trajectoryTable = new QTableWidget();
    trajectoryTable->setColumnCount(4);
    trajectoryTable->setHorizontalHeaderLabels(QStringList() << "D" << "O" << "T" << "TO");
    
    // Apply table styling - exact same as main window
    trajectoryTable->setStyleSheet(
        "QTableWidget { \n"
        "  gridline-color: rgb(100, 100, 100);\n"
        "  background-color: rgb(50, 50, 50);\n"
        "}\n"
        "QTableWidget::item {\n"
        "  padding: 5px;\n"
        "  border: 1px solid rgb(100, 100, 100);\n"
        "  background-color: rgb(50, 50, 50);\n"
        "  color: rgb(200, 200, 200);\n"
        "}"
    );
    
    trajectoryTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background-color: rgb(85, 87, 83); "
        "color: white; font-weight: bold; padding: 5px; }"
    );
    
    trajectoryTable->verticalHeader()->setVisible(false);
    trajectoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    trajectoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    trajectoryTable->setSelectionMode(QAbstractItemView::SingleSelection);
    trajectoryTable->setShowGrid(true);
    trajectoryTable->setGridStyle(Qt::SolidLine);
    trajectoryTable->setFrameShape(QFrame::NoFrame);
    trajectoryTable->setFrameShadow(QFrame::Raised);
    trajectoryTable->setLineWidth(0);
    
    rightLayout->addWidget(trajectoryTable, 1);
    
    // Set equal stretch factors for top and bottom
    rightLayout->setStretch(0, 1);
    rightLayout->setStretch(1, 1);
    
    mainLayout->addLayout(rightLayout, 1);
    setLayout(mainLayout);
}

void TrajectoryView::populateDummyData()
{
    // Populate Tube list
    for (int i = 1001; i <= 1010; i++) {
        tubeList->addItem(QString::number(i));
    }
    
    // Populate trajectory table with dummy data
    for (int row = 0; row < 5; row++) {
        trajectoryTable->insertRow(row);
        
        trajectoryTable->setItem(row, 0, new QTableWidgetItem(QString::number(100 + row * 50)));
        trajectoryTable->setItem(row, 1, new QTableWidgetItem(QString::number(200 + row * 100)));
        trajectoryTable->setItem(row, 2, new QTableWidgetItem(QString::number(300 + row * 75)));
        trajectoryTable->setItem(row, 3, new QTableWidgetItem(QString::number(400 + row * 125)));
    }
    
    trajectoryTable->verticalHeader()->setDefaultSectionSize(23);
}
