#include "eventview.h"
#include <QHeaderView>

EventView::EventView(QWidget *parent)
    : QWidget(parent), eventTable(nullptr), layout(nullptr)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    eventTable = new QTableWidget(this);
    eventTable->setObjectName("eventTable");
    setupTable();
    layout->addWidget(eventTable);
    
    // Connect row click signal
    connect(eventTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        emit rowClicked(row);
    });
    
    setLayout(layout);
}

EventView::~EventView()
{
}

void EventView::setupTable()
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

void EventView::addEvent(const QString &sender, const QString &systemTime,
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

void EventView::clearEvents()
{
    eventTable->setRowCount(0);
}
