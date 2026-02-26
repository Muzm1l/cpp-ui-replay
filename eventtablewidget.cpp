#include "eventtablewidget.h"
#include <QHeaderView>

EventTableWidget::EventTableWidget(QWidget *parent)
    : QWidget(parent), eventTable(nullptr), layout(nullptr)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    eventTable = new QTableWidget(this);
    setupTable();
    layout->addWidget(eventTable);
    
    // Connect row click signal
    connect(eventTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        emit rowClicked(row);
    });
    
    setLayout(layout);
}

EventTableWidget::~EventTableWidget()
{
}

void EventTableWidget::setupTable()
{
    // Set column count and headers
    eventTable->setColumnCount(6);
    eventTable->setHorizontalHeaderLabels(
        QStringList() << "SENDER" 
                      << "System Time" 
                      << "Engagement Time" 
                      << "Torpedo Time" 
                      << "msg type" 
                      << "msg label"
    );
    
    // Hide row headers
    eventTable->verticalHeader()->setVisible(false);
    
    // Apply the same style as geographical information table
    eventTable->setStyleSheet(
        "QTableWidget { \n"
        "  gridline-color: rgb(0, 103, 238);\n"
        "  background-color: rgb(0, 0, 0);\n"
        "}\n"
        "QTableWidget::item {\n"
        "  padding: 5px;\n"
        "  border: 1px solid rgb(0, 60, 255);\n"
        "  background-color: rgb(100, 100, 100);\n"
        "  color: rgb(255, 255, 255);\n"
        "}\n"
        "QTableWidget::item:alternate {\n"
        "  background-color: rgb(0, 0, 0);\n"
        "}"
    );
    
    eventTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background-color: rgb(85, 87, 83); "
        "color: white; font-weight: bold; padding: 5px; }"
    );
    
    eventTable->setFrameShape(QFrame::NoFrame);
    eventTable->setFrameShadow(QFrame::Raised);
    eventTable->setLineWidth(0);
    eventTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    eventTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    eventTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
    eventTable->setAutoScrollMargin(13);
    eventTable->setDragDropMode(QAbstractItemView::InternalMove);
    
    eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    eventTable->setSelectionMode(QAbstractItemView::SingleSelection);
    eventTable->setShowGrid(true);
    eventTable->setAlternatingRowColors(true);
    eventTable->setGridStyle(Qt::SolidLine);
    
    // Set column widths
    eventTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    eventTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set row height
    eventTable->verticalHeader()->setDefaultSectionSize(23);
    
    // Set minimum size
    eventTable->setMinimumSize(400, 300);
}

void EventTableWidget::addEvent(const QString &sender, const QString &systemTime,
                               const QString &engagementTime, const QString &torpedoTime,
                               const QString &msgType, const QString &msgLabel)
{
    int row = eventTable->rowCount();
    eventTable->insertRow(row);
    
    // Create table items with data
    eventTable->setItem(row, 0, new QTableWidgetItem(sender));
    eventTable->setItem(row, 1, new QTableWidgetItem(systemTime));
    eventTable->setItem(row, 2, new QTableWidgetItem(engagementTime));
    eventTable->setItem(row, 3, new QTableWidgetItem(torpedoTime));
    eventTable->setItem(row, 4, new QTableWidgetItem(msgType));
    eventTable->setItem(row, 5, new QTableWidgetItem(msgLabel));
}

void EventTableWidget::clearEvents()
{
    eventTable->setRowCount(0);
}
