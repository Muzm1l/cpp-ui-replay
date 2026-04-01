#include "trajectorytable.h"
#include <QHeaderView>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QTimer>

TableColumn::TableColumn(const QString &headerName)
    : m_headerName(headerName)
{
}

void TableColumn::applyToTable(QTableWidget *table, int columnIndex) {
    if (!table || columnIndex < 0 || columnIndex >= table->columnCount()) {
        return;
    }
    
    // Set header text
    QTableWidgetItem *headerItem = new QTableWidgetItem(m_headerName);
    table->setHorizontalHeaderItem(columnIndex, headerItem);
    
    // Set column width if specified
    if (m_width > 0) {
        table->setColumnWidth(columnIndex, m_width);
    }
    
    // Apply resize mode
    QHeaderView::ResizeMode qtResizeMode = QHeaderView::Fixed;
    switch (m_resizeMode) {
        case ResizeMode::Fixed:
            qtResizeMode = QHeaderView::Fixed;
            break;
        case ResizeMode::Stretch:
            qtResizeMode = QHeaderView::Stretch;
            break;
        case ResizeMode::Interactive:
            qtResizeMode = QHeaderView::Interactive;
            break;
    }
    table->horizontalHeader()->setSectionResizeMode(columnIndex, qtResizeMode);
    
    // Populate column data if provided
    if (!m_rowData.isEmpty()) {
        for (int row = 0; row < m_rowData.count() && row < table->rowCount(); ++row) {
            QTableWidgetItem *item = new QTableWidgetItem(m_rowData[row]);
            
            // Apply alignment
            item->setTextAlignment(m_alignment);
            
            // Apply word wrap if enabled
            if (m_wordWrap) {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }
            
            // Apply styling
            if (m_bgColor.isValid()) {
                item->setBackground(m_bgColor);
            }
            if (m_textColor.isValid()) {
                item->setForeground(m_textColor);
            }
            
            // Apply font styling
            if (m_fontBold) {
                QFont font = item->font();
                font.setBold(true);
                item->setFont(font);
            }
            
            // Set editable flag
            if (!m_editable) {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
            
            table->setItem(row, columnIndex, item);
        }
    }
}

TrajectoryTable::TrajectoryTable(QWidget *parent)

    : QWidget(parent)
{
    setObjectName("trajectoryTableGroup");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Add close button
    closeButton = new QPushButton("✕", this);
    closeButton->setObjectName("miniCloseButton");
    closeButton->setFixedSize(24, 24);
    closeButton->move(0, 0);
    closeButton->raise();
    connect(closeButton, &QPushButton::clicked, this, [this]() { emit closeRequested(); this->hide(); });

    QHBoxLayout *tablesLayout = new QHBoxLayout();
    tablesLayout->setContentsMargins(0, 0, 0, 0);
    tablesLayout->setSpacing(0);

    mainLayout->addLayout(tablesLayout, 1);

    // Split previous 4 columns into 4 independent single-column tables.
    m_columnTables.append(createColumnTable("Date", QStringList() << "Date" << "Time", false, 4, 1));
    m_columnTables.append(createColumnTable("Ownship", QStringList() << "Heading" << "Speed" << "Depth", false, 4, 3));
    m_columnTables.append(createColumnTable("Target", QStringList() << "Heading" << "Speed" << "Bearing", false, 4, 3));

    QTableWidget *torpedoTable = createColumnTable(
        "Torpedo",
        QStringList() << "Heading" << "Speed" << "Bearing" << "Depth" << "WG Type" << "Torpedo Phase",
        true,
        6,
        3
    );
    torpedoTable->setObjectName("torpedoSubTable");
    m_torpedoTable = torpedoTable;

    QScrollBar *internalScrollBar = torpedoTable->verticalScrollBar();
    internalScrollBar->setObjectName("torpedoVerticalScrollBar");
    torpedoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_torpedoExternalScrollBar = new QScrollBar(Qt::Vertical, torpedoTable);
    m_torpedoExternalScrollBar->setObjectName("torpedoExternalScrollBar");
    m_columnTables.append(torpedoTable);

    connect(internalScrollBar, &QScrollBar::rangeChanged, this,
            [this](int, int) { syncTorpedoExternalScrollbar(); });
    connect(internalScrollBar, &QScrollBar::valueChanged,
            m_torpedoExternalScrollBar, &QScrollBar::setValue);
    connect(m_torpedoExternalScrollBar, &QScrollBar::valueChanged,
            internalScrollBar, &QScrollBar::setValue);
    connect(torpedoTable->horizontalHeader(), &QHeaderView::geometriesChanged,
            this, &TrajectoryTable::adjustTorpedoScrollbarGeometry);

    for (QTableWidget *table : m_columnTables) {
        tablesLayout->addWidget(table, 1);
    }

    updateColumnWidths();
    QTimer::singleShot(0, this, [this]() { syncTorpedoExternalScrollbar(); });
}

QTableWidget *TrajectoryTable::createColumnTable(const QString &header, const QStringList &rows, bool enableVerticalScroll, int totalRows, int subColumnCount) {
    QTableWidget *table = new QTableWidget(this);
    table->setObjectName("trajectorySubTable");
    table->setProperty("multiColumnTable", subColumnCount > 1);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    table->setColumnCount(subColumnCount);
    table->setRowCount(totalRows);

    QStringList headers;
    if (subColumnCount == 1) {
        headers << header.toUpper();
    } else {
        const int middleIndex = subColumnCount / 2;
        for (int i = 0; i < subColumnCount; ++i) {
            headers << (i == middleIndex ? header.toUpper() : "");
        }
    }
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);

    table->setShowGrid(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->verticalHeader()->setVisible(false);
    for (int col = 0; col < subColumnCount; ++col) {
        table->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Stretch);
    }
    table->setVerticalScrollBarPolicy(enableVerticalScroll ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const int rowHeight = 34;
    for (int row = 0; row < totalRows; ++row) {
        table->setRowHeight(row, rowHeight);
        for (int col = 0; col < subColumnCount; ++col) {
            QString cellText;
            if (col == 0 && row < rows.size()) {
                cellText = rows[row];
            } else if (subColumnCount > 1 && col == 1) {
                cellText = "VALUE";
            } else if (subColumnCount > 2 && col == 2) {
                cellText = "UNITS";
            }
            QTableWidgetItem *item = new QTableWidgetItem(cellText);
            if (header == "Date" && col == 0) {
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            } else if (col == 1 || col == 2) {
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }
            table->setItem(row, col, item);
        }
    }

    // Show only 4 rows per table; overflow is handled by that table's own scrollbar.
    const int maxVisibleRows = 4;
    int tableHeight = table->horizontalHeader()->height() + (maxVisibleRows * rowHeight) + 2;
    table->setMinimumHeight(tableHeight);
    table->setMaximumHeight(tableHeight);

    return table;
}

void TrajectoryTable::updateColumnWidths() {
    if (m_columnTables.isEmpty()) {
        return;
    }

    int availableWidth = width();
    if (availableWidth <= 0) {
        return;
    }

    const int firstTableWidth = 200;
    m_columnTables[0]->setMinimumWidth(firstTableWidth);
    m_columnTables[0]->setMaximumWidth(firstTableWidth);

    int remainingTables = m_columnTables.size() - 1;
    if (remainingTables <= 0) {
        return;
    }

    int remainingWidth = qMax(0, availableWidth - firstTableWidth);
    int otherTableWidth = qMax(80, remainingWidth / remainingTables);
    for (int i = 1; i < m_columnTables.size(); ++i) {
        m_columnTables[i]->setMinimumWidth(otherTableWidth);
        m_columnTables[i]->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}

void TrajectoryTable::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    updateColumnWidths();
    adjustTorpedoScrollbarGeometry();
    if (closeButton) {
        closeButton->move(0, 0);
        closeButton->raise();
    }
}

void TrajectoryTable::adjustTorpedoScrollbarGeometry() {
    if (!m_torpedoTable || !m_torpedoExternalScrollBar) {
        return;
    }

    const int headerHeight = m_torpedoTable->horizontalHeader()->height();
    const int scrollBarWidth = m_torpedoExternalScrollBar->sizeHint().width();

    const int x = m_torpedoTable->width() - scrollBarWidth - 1;
    const int y = headerHeight + 1;
    const int height = qMax(0, m_torpedoTable->height() - headerHeight - 2);

    m_torpedoExternalScrollBar->setGeometry(x, y, scrollBarWidth, height);
}

void TrajectoryTable::syncTorpedoExternalScrollbar() {
    if (!m_torpedoTable || !m_torpedoExternalScrollBar) {
        return;
    }

    QScrollBar *internalScrollBar = m_torpedoTable->verticalScrollBar();
    if (!internalScrollBar) {
        return;
    }

    m_torpedoExternalScrollBar->setRange(internalScrollBar->minimum(), internalScrollBar->maximum());
    m_torpedoExternalScrollBar->setPageStep(internalScrollBar->pageStep());
    m_torpedoExternalScrollBar->setSingleStep(internalScrollBar->singleStep());
    m_torpedoExternalScrollBar->setValue(internalScrollBar->value());
    m_torpedoExternalScrollBar->setVisible(internalScrollBar->maximum() > internalScrollBar->minimum());
    adjustTorpedoScrollbarGeometry();
}
