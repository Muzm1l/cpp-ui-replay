#ifndef TRAJECTORYTABLE_H
#define TRAJECTORYTABLE_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVector>
#include "simulator.h"

/**
 * @class TableColumn
 * @brief Reusable table column component with customizable properties
 * 
 * This class encapsulates a table column with all its properties and behaviors.
 * It can be customized and reused across different tables and layouts.
 */
class TableColumn {
public:
    enum ResizeMode {
        Fixed,      // Column has fixed width
        Stretch,    // Column stretches to fill available space
        Interactive // User can manually resize column
    };
    
    /**
     * @brief Constructor for creating a customizable table column
     * @param headerName The header label for this column
     * @param parent Parent widget (optional)
     */
    explicit TableColumn(const QString &headerName);
    
    // Getter methods
    QString getHeaderName() const { return m_headerName; }
    int getWidth() const { return m_width; }
    ResizeMode getResizeMode() const { return m_resizeMode; }
    Qt::Alignment getAlignment() const { return m_alignment; }
    QStringList getRowData() const { return m_rowData; }
    bool isScrollable() const { return m_isScrollable; }
    Qt::TextElideMode getElideMode() const { return m_elideMode; }
    
    // Setter methods for customization
    TableColumn& setWidth(int width) { m_width = width; return *this; }
    TableColumn& setResizeMode(ResizeMode mode) { m_resizeMode = mode; return *this; }
    TableColumn& setAlignment(Qt::Alignment align) { m_alignment = align; return *this; }
    TableColumn& setScrollable(bool scrollable) { m_isScrollable = scrollable; return *this; }
    TableColumn& setElideMode(Qt::TextElideMode mode) { m_elideMode = mode; return *this; }
    TableColumn& setRowData(const QStringList &data) { m_rowData = data; return *this; }
    TableColumn& setBackgroundColor(const QColor &color) { m_bgColor = color; return *this; }
    TableColumn& setTextColor(const QColor &color) { m_textColor = color; return *this; }
    TableColumn& setFontBold(bool bold) { m_fontBold = bold; return *this; }
    TableColumn& setEditable(bool editable) { m_editable = editable; return *this; }
    TableColumn& setWordWrap(bool wrap) { m_wordWrap = wrap; return *this; }
    
    // Get styling properties
    QColor getBackgroundColor() const { return m_bgColor; }
    QColor getTextColor() const { return m_textColor; }
    bool isFontBold() const { return m_fontBold; }
    bool isEditable() const { return m_editable; }
    bool hasWordWrap() const { return m_wordWrap; }
    
    /**
     * @brief Apply this column configuration to a QTableWidget
     * @param table The table widget to apply to
     * @param columnIndex The column index
     */
    void applyToTable(QTableWidget *table, int columnIndex);
    
private:
    QString m_headerName;
    int m_width = -1;
    ResizeMode m_resizeMode = Fixed;
    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    QStringList m_rowData;
    bool m_isScrollable = false;
    Qt::TextElideMode m_elideMode = Qt::ElideRight;
    QColor m_bgColor;
    QColor m_textColor;
    bool m_fontBold = false;
    bool m_editable = false;
    bool m_wordWrap = false;
};

/**
 * @class ColumnManager
 * @brief Manages multiple TableColumn instances for easy table construction
 */
class ColumnManager {
public:
    ColumnManager() = default;
    
    void addColumn(const TableColumn &column) {
        m_columns.append(column);
    }
    
    void addColumn(const QString &headerName) {
        m_columns.append(TableColumn(headerName));
    }
    
    TableColumn& getColumn(int index) {
        return m_columns[index];
    }
    
    int columnCount() const {
        return m_columns.count();
    }
    
    void applyToTable(QTableWidget *table) {
        for (int i = 0; i < m_columns.count(); ++i) {
            m_columns[i].applyToTable(table, i);
        }
    }
    
    void clear() {
        m_columns.clear();
    }
    
private:
    QVector<TableColumn> m_columns;
};

class TrajectoryTable : public QWidget {
    Q_OBJECT
public:
    explicit TrajectoryTable(QWidget *parent = nullptr);
    void updateStampedInfo(const Simulator::Sample& sample, std::size_t stampedIndex);
    void clearStampedInfo();
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    
signals:
    void closeRequested();
private:
    QTableWidget *createColumnTable(const QString &header, const QStringList &rows, int totalRows, int subColumnCount);
    void updateColumnWidths();
    
    QPushButton *closeButton = nullptr;
    QVector<QTableWidget*> m_columnTables;
    QTableWidget *m_torpedoTable = nullptr;

    QTableWidget *m_dateTable = nullptr;
    QTableWidget *m_ownshipTable = nullptr;
    QTableWidget *m_targetTable = nullptr;

    void setCellText(QTableWidget *table, int row, int col, const QString &text);
};

#endif // TRAJECTORYTABLE_H
