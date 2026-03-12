#include "trajectorytable.h"
#include <QHeaderView>

TrajectoryTable::TrajectoryTable(QWidget *parent)
        
    : QTableWidget(parent)
{
    // Add close button
    closeButton = new QPushButton("✕", this);
    closeButton->setFixedSize(24, 24);
    closeButton->setStyleSheet("QPushButton { background: #e70a0a; border: 1px solid #888; font-weight: bold; color: #ffffff; } QPushButton:hover { background: rgb(250, 0, 0); }");
    closeButton->move(0, 0);
    closeButton->raise();
    connect(closeButton, &QPushButton::clicked, this, [this]() { emit closeRequested(); this->hide(); });
    setShowGrid(true);
    setStyleSheet("QTableWidget { border: 1px solid #444; gridline-color: #444; } QHeaderView::section { border: 1px solid #444; } QTableWidget::item { border-right: 1px solid #444; }");
        int rowHeight = 34;
    for (int row = 0; row < 6; ++row) {
        setRowHeight(row, rowHeight); // Header row height for all
    }
        setFixedHeight(horizontalHeader()->height() + 6 * rowHeight + 2); // Table height fits 6 rows plus header
        setStyleSheet("QTableWidget { border: 1px solid #fffefe; gridline-color: #ffffff; padding: 0px; margin: 0px; } QHeaderView::section { border: 1px solid #ffffff; padding: 0px; margin: 0px; } QTableWidget::item { border-right: 1px solid #fffdfd; padding-top: 2px; padding-bottom: -8px; margin: 0px; }");
    verticalHeader()->setVisible(false);

    // Set up table properties, columns, etc.
    setColumnCount(4);
    setRowCount(6);
    setHorizontalHeaderLabels({"Date", "Ownship", "Target", "Torpedo"});
    setItem(0, 0, new QTableWidgetItem("Date"));
    setItem(1, 0, new QTableWidgetItem("Time"));
    setItem(0, 1, new QTableWidgetItem("Heading"));
    setItem(1, 1, new QTableWidgetItem("Speed"));
    setItem(2, 1, new QTableWidgetItem("Depth"));
    setItem(0, 2, new QTableWidgetItem("Heading"));
    setItem(1, 2, new QTableWidgetItem("Speed"));
    setItem(2, 2, new QTableWidgetItem("Bearing"));
    setItem(0, 3, new QTableWidgetItem("Heading"));
    setItem(1, 3, new QTableWidgetItem("Speed"));
    setItem(2, 3, new QTableWidgetItem("Bearing"));
    setItem(3, 3, new QTableWidgetItem("Depth"));
    setItem(4, 3, new QTableWidgetItem("WG Type"));
    setItem(5, 3, new QTableWidgetItem("Torpedo Phase"));

    // Set column widths to visually match the example
    setColumnWidth(0, 120);   // Date
    setColumnWidth(1, 220);  // Ownship
    setColumnWidth(2, 220);  // Target
    setColumnWidth(3, 260);  // Torpedo
    for (int col = 0; col < columnCount(); ++col) {
        if (item(0, col)) item(0, col)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        if (item(0, col)) item(0, col)->setFlags(item(0, col)->flags() & ~Qt::ItemIsEditable);
        // Word wrap is not supported directly on QTableWidgetItem; remove this line.
    }
}
