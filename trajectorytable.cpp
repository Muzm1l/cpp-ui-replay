#include "trajectorytable.h"
#include <QHeaderView>
#include <QSizePolicy>
#include <QResizeEvent>
#include <QColor>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QStringList>
#include <QtMath>
#include <QEvent>

namespace {

double normalizeBearingDeg(double angleDeg)
{
    double normalized = std::fmod(angleDeg, 360.0);
    if (normalized < 0.0) {
        normalized += 360.0;
    }
    return normalized;
}

double bearingFromOwnshipDeg(double ownX, double ownY, double targetX, double targetY)
{
    const double dx = targetX - ownX;
    const double dy = targetY - ownY;
    const double angle = qRadiansToDegrees(std::atan2(dx, dy));
    return normalizeBearingDeg(angle);
}

QString fmt1(double value)
{
    return QString::number(value, 'f', 1);
}

} // namespace

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

    const int totalVisibleRows = 7;
    m_dateTable = createColumnTable("Date", QStringList() << "Date" << "Time", totalVisibleRows, 1);
    m_columnTables.append(m_dateTable);
    m_ownshipTable = createColumnTable("Ownship", QStringList() << "Heading" << "Speed" << "Depth", totalVisibleRows, 3);
    m_columnTables.append(m_ownshipTable);
    m_targetTable = createColumnTable("Target", QStringList() << "Heading" << "Speed" << "Bearing", totalVisibleRows, 3);
    m_columnTables.append(m_targetTable);

    QTableWidget *torpedoTable = createColumnTable(
        "Torpedo",
        QStringList() << "Heading" << "Speed" << "Bearing" << "Depth" << "WG Type" << "Torpedo Phase",
        totalVisibleRows,
        3
    );
    torpedoTable->setObjectName("torpedoSubTable");
    m_torpedoTable = torpedoTable;
    m_columnTables.append(torpedoTable);

    for (QTableWidget *table : m_columnTables) {
        tablesLayout->addWidget(table, 1);
        table->installEventFilter(this);
        table->viewport()->installEventFilter(this);
    }

    int uniformTableHeight = 0;
    for (QTableWidget *table : m_columnTables) {
        uniformTableHeight = qMax(uniformTableHeight, table->minimumHeight());
    }
    for (QTableWidget *table : m_columnTables) {
        table->setMinimumHeight(uniformTableHeight);
        table->setMaximumHeight(uniformTableHeight);
    }

    updateColumnWidths();
}

QTableWidget *TrajectoryTable::createColumnTable(const QString &header, const QStringList &rows, int totalRows, int subColumnCount) {
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
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    const int rowHeight = table->fontMetrics().height() + 1;
    for (int row = 0; row < totalRows; ++row) {
        table->setRowHeight(row, rowHeight);
        for (int col = 0; col < subColumnCount; ++col) {
            QString cellText;
            if (col == 0 && row < rows.size()) {
                cellText = rows[row];
            } else if (subColumnCount > 1 && col == 1) {
                cellText = (row < rows.size()) ? "VALUE" : "";
            } else if (subColumnCount > 2 && col == 2) {
                cellText = (row < rows.size()) ? "UNITS" : "";
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

    int tableHeight = table->horizontalHeader()->height() + (totalRows * rowHeight) + 2;
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
    if (closeButton) {
        closeButton->move(0, 0);
        closeButton->raise();
    }
}

bool TrajectoryTable::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event && event->type() == QEvent::Wheel) {
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void TrajectoryTable::setCellText(QTableWidget *table, int row, int col, const QString &text)
{
    if (!table || row < 0 || col < 0 || row >= table->rowCount() || col >= table->columnCount()) {
        return;
    }

    QTableWidgetItem *item = table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem();
        table->setItem(row, col, item);
    }
    item->setText(text);
}

void TrajectoryTable::updateStampedInfo(const Simulator::Sample& sample, std::size_t stampedIndex)
{
    const QString timestamp = QString::fromStdString(sample.timestamp);
    const QStringList dateTimeParts = timestamp.split(' ');
    const QString datePart = dateTimeParts.size() > 0 ? dateTimeParts[0] : "N/A";
    const QString timePart = dateTimeParts.size() > 1 ? dateTimeParts[1] : "N/A";

    setCellText(m_dateTable, 0, 0, QString("Date: %1").arg(datePart));
    setCellText(m_dateTable, 1, 0, QString("Time: %1").arg(timePart));

    setCellText(m_ownshipTable, 0, 1, fmt1(sample.ownHeadingDeg));
    setCellText(m_ownshipTable, 0, 2, "deg");
    setCellText(m_ownshipTable, 1, 1, fmt1(sample.ownSpeedKnots));
    setCellText(m_ownshipTable, 1, 2, "kn");
    setCellText(m_ownshipTable, 2, 1, "N/A");
    setCellText(m_ownshipTable, 2, 2, "m");

    const double targetBearingDeg = bearingFromOwnshipDeg(sample.ownX, sample.ownY, sample.targetX, sample.targetY);
    setCellText(m_targetTable, 0, 1, fmt1(sample.targetHeadingDeg));
    setCellText(m_targetTable, 0, 2, "deg");
    setCellText(m_targetTable, 1, 1, fmt1(sample.targetSpeedKnots));
    setCellText(m_targetTable, 1, 2, "kn");
    setCellText(m_targetTable, 2, 1, fmt1(targetBearingDeg));
    setCellText(m_targetTable, 2, 2, "deg");

    const bool torpedoLaunched = sample.torpedoSpeedKnots > 0.0;
    const double torpedoBearingDeg = bearingFromOwnshipDeg(sample.ownX, sample.ownY, sample.torpedoX, sample.torpedoY);
    const QString guidancePhase = !torpedoLaunched ? "Standby" : (stampedIndex < 90 ? "Guided" : "Terminal");
    const QString torpedoPhase = !torpedoLaunched ? "Pre-Launch" : "In-Run";

    setCellText(m_torpedoTable, 0, 1, torpedoLaunched ? fmt1(sample.torpedoHeadingDeg) : "N/A");
    setCellText(m_torpedoTable, 0, 2, "deg");
    setCellText(m_torpedoTable, 1, 1, torpedoLaunched ? fmt1(sample.torpedoSpeedKnots) : "0.0");
    setCellText(m_torpedoTable, 1, 2, "kn");
    setCellText(m_torpedoTable, 2, 1, torpedoLaunched ? fmt1(torpedoBearingDeg) : "N/A");
    setCellText(m_torpedoTable, 2, 2, "deg");
    setCellText(m_torpedoTable, 3, 1, "N/A");
    setCellText(m_torpedoTable, 3, 2, "m");
    setCellText(m_torpedoTable, 4, 1, guidancePhase);
    setCellText(m_torpedoTable, 4, 2, "-");
    setCellText(m_torpedoTable, 5, 1, torpedoPhase);
    setCellText(m_torpedoTable, 5, 2, "-");
}

void TrajectoryTable::clearStampedInfo()
{
    if (!m_dateTable || !m_ownshipTable || !m_targetTable || !m_torpedoTable) {
        return;
    }

    setCellText(m_dateTable, 0, 0, "Date");
    setCellText(m_dateTable, 1, 0, "Time");

    const QList<QTableWidget*> valueTables{m_ownshipTable, m_targetTable, m_torpedoTable};
    for (QTableWidget *table : valueTables) {
        for (int row = 0; row < table->rowCount(); ++row) {
            setCellText(table, row, 1, "");
            setCellText(table, row, 2, "");
        }
    }
}
