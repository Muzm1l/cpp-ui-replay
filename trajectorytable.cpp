#include "trajectorytable.h"
#include <QHeaderView>
#include <QSizePolicy>

TrajectoryTable::TrajectoryTable(QWidget *parent)
        
    : QTableWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add close button
    closeButton = new QPushButton("✕", this);
    closeButton->setFixedSize(24, 24);
    closeButton->setStyleSheet("QPushButton { background: #e70a0a; border: 1px solid #888; font-weight: bold; color: #ffffff; } QPushButton:hover { background: rgb(250, 0, 0); }");
    closeButton->move(0, 0);
    closeButton->raise();
    connect(closeButton, &QPushButton::clicked, this, [this]() { emit closeRequested(); this->hide(); });
    // We want only vertical separators (no horizontal grid lines).
    // Disable the built-in grid and draw separators via stylesheet.
    setShowGrid(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet(
        "QTableWidget { border: 1px solid #ffffff; background: #5a5a5a; color: white; }\n"
        "QHeaderView::section {"
        "  border-right: 1px solid #ffffff;"
        "  border-bottom: 1px solid #ffffff;"
        "  padding: 6px 8px;"
        "  background: #2b7fb8;"
        "  color: white;"
        "}\n"
        "QTableWidget::item {"
        "  border-right: 1px solid #ffffff;"
        "  border-bottom: 0px;"
        "  padding: 6px 8px;"
        "  color: white;"
        "  background: #5a5a5a;"
        "}\n"
    );

    int rowHeight = 34;
    for (int row = 0; row < 6; ++row) {
        setRowHeight(row, rowHeight); // Header row height for all
    }
    verticalHeader()->setVisible(false);

    // Set up table properties, columns, etc.
    setColumnCount(4);
    setRowCount(6);
    setHorizontalHeaderLabels({"Date", "Ownship", "Target", "Torpedo"});
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

    for (int col = 0; col < columnCount(); ++col) {
        if (item(0, col)) item(0, col)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        if (item(0, col)) item(0, col)->setFlags(item(0, col)->flags() & ~Qt::ItemIsEditable);
        // Word wrap is not supported directly on QTableWidgetItem; remove this line.
    }
}
