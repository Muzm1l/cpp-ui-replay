#include "geographicalinfo.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <qaction.h>

GeographicalInfo::GeographicalInfo(QWidget *parent)
    : QWidget(parent)
    , mainLayout(nullptr)
    , headerLabel(nullptr)
    , geoTable(nullptr)
{
    setupUI();
}

GeographicalInfo::~GeographicalInfo()
{
    // Qt handles cleanup of child widgets automatically
}

void GeographicalInfo::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header Label
    headerLabel = new QLabel("Geographical Information");
    headerLabel->setObjectName("geoHeaderLabel");
    headerLabel->setMinimumHeight(30);
    headerLabel->setMaximumHeight(30);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    
    // Setup table
    setupTable();
}

void GeographicalInfo::setupTable()
{
    geoTable = new QTableWidget(this);
    geoTable->setObjectName("geoTable");
    geoTable->setColumnCount(2);
    geoTable->setRowCount(7);
    geoTable->setFrameShape(QFrame::NoFrame);
    geoTable->setFrameShadow(QFrame::Raised);
    geoTable->setLineWidth(0);
    geoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    geoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    geoTable->setShowGrid(true);
    geoTable->setGridStyle(Qt::SolidLine);
    geoTable->horizontalHeader()->setVisible(false);
    geoTable->verticalHeader()->setVisible(false);
    
    // Set column widths
    geoTable->setColumnWidth(0, 225);
    geoTable->setColumnWidth(1, 175);
    
    // Setup row labels
    QStringList geoHeaders = {
        "Current Selected Time",
        "Ownship / Torpedo Bearing",
        "Ownship / Torpedo Range",
        "Ownship / Target Bearing",
        "Ownship / Target Range",
        "Torpedo / Target Bearing",
        "Torpedo / Target Range"
    };
    
    for (int i = 0; i < 7; ++i) {
        QTableWidgetItem *labelItem = new QTableWidgetItem(geoHeaders[i]);
        labelItem->setFlags(labelItem->flags() & ~Qt::ItemIsEditable);
        geoTable->setItem(i, 0, labelItem);
        
        QTableWidgetItem *valueItem = new QTableWidgetItem("--");
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        geoTable->setItem(i, 1, valueItem);
    }
    
    mainLayout->addWidget(geoTable);
}

void GeographicalInfo::updateValue(int row, const QString& value)
{
    if (row >= 0 && row < geoTable->rowCount()) {
        QTableWidgetItem *item = geoTable->item(row, 1);
        if (item) {
            item->setText(value);
        }
    }
}

void GeographicalInfo::setCurrentTime(float time)
{
    updateValue(ROW_CURRENT_TIME, QString::number(time, 'f', 2) + " s");
    emit dataChanged();
}

void GeographicalInfo::setOwnshipTorpedoBearing(float bearing)
{
    updateValue(ROW_OWNSHIP_TORPEDO_BEARING, QString::number(bearing, 'f', 1) + "°");
    emit dataChanged();
}

void GeographicalInfo::setOwnshipTorpedoRange(float range)
{
    updateValue(ROW_OWNSHIP_TORPEDO_RANGE, QString::number(range, 'f', 1) + " m");
    emit dataChanged();
}

void GeographicalInfo::setOwnshipTargetBearing(float bearing)
{
    updateValue(ROW_OWNSHIP_TARGET_BEARING, QString::number(bearing, 'f', 1) + "°");
    emit dataChanged();
}

void GeographicalInfo::setOwnshipTargetRange(float range)
{
    updateValue(ROW_OWNSHIP_TARGET_RANGE, QString::number(range, 'f', 1) + " m");
    emit dataChanged();
}

void GeographicalInfo::setTorpedoTargetBearing(float bearing)
{
    updateValue(ROW_TORPEDO_TARGET_BEARING, QString::number(bearing, 'f', 1) + "°");
    emit dataChanged();
}

void GeographicalInfo::setTorpedoTargetRange(float range)
{
    updateValue(ROW_TORPEDO_TARGET_RANGE, QString::number(range, 'f', 1) + " m");
    emit dataChanged();
}

void GeographicalInfo::updateGeoData(const GeoData& data)
{
    setCurrentTime(data.currentTime);
    setOwnshipTorpedoBearing(data.ownshipTorpedoBearing);
    setOwnshipTorpedoRange(data.ownshipTorpedoRange);
    setOwnshipTargetBearing(data.ownshipTargetBearing);
    setOwnshipTargetRange(data.ownshipTargetRange);
    setTorpedoTargetBearing(data.torpedoTargetBearing);
    setTorpedoTargetRange(data.torpedoTargetRange);
}

void GeographicalInfo::clearData()
{
    for (int i = 0; i < geoTable->rowCount(); ++i) {
        updateValue(i, "--");
    }
    emit dataChanged();
}

void GeographicalInfo::setHeaderVisible(bool visible)
{
    headerLabel->setVisible(visible);
}

void GeographicalInfo::setHeaderText(const QString& text)
{
    headerLabel->setText(text);
}

void GeographicalInfo::setFixedWidthMode(bool enabled, int width)
{
    if (enabled) {
        setMinimumWidth(width);
        setMaximumWidth(width);
        geoTable->setMinimumWidth(width);
        geoTable->setMaximumWidth(width);
        headerLabel->setMaximumWidth(width);
    } else {
        setMinimumWidth(0);
        setMaximumWidth(QWIDGETSIZE_MAX);
        geoTable->setMinimumWidth(0);
        geoTable->setMaximumWidth(QWIDGETSIZE_MAX);
        headerLabel->setMaximumWidth(QWIDGETSIZE_MAX);
    }
}
