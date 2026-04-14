#include "geographicalinfo.h"
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>
#include <qaction.h>
#include <QSignalBlocker>
#include <cmath>

namespace {

constexpr float kPi = 3.14159265358979323846f;

float normalizeBearing(float bearingDeg)
{
    while (bearingDeg < 0.0f) {
        bearingDeg += 360.0f;
    }
    while (bearingDeg >= 360.0f) {
        bearingDeg -= 360.0f;
    }
    return bearingDeg;
}

float calculateBearing(float fromX, float fromY, float toX, float toY)
{
    const float dx = toX - fromX;
    const float dy = toY - fromY;
    const float rad = std::atan2(dx, dy);
    return normalizeBearing(rad * (180.0f / kPi));
}

float calculateRange(float fromX, float fromY, float toX, float toY)
{
    const float dx = toX - fromX;
    const float dy = toY - fromY;
    return std::sqrt((dx * dx) + (dy * dy));
}

} // namespace

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
    geoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    geoTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    geoTable->setEditTriggers(QAbstractItemView::DoubleClicked |
                              QAbstractItemView::SelectedClicked |
                              QAbstractItemView::EditKeyPressed |
                              QAbstractItemView::AnyKeyPressed);
    
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
        if (i == ROW_CURRENT_TIME) {
            valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);
            valueItem->setToolTip("Enter time as hh:mm:ss or seconds");
        } else {
            valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        }
        geoTable->setItem(i, 1, valueItem);
    }

    connect(geoTable, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item) {
        if (!item || item->row() != ROW_CURRENT_TIME || item->column() != 1) {
            return;
        }

        QString text = item->text().trimmed();
        float currentTime = timeFormatToSeconds(text);
        if (currentTime < 0) {
            return;
        }

        const QSignalBlocker blocker(geoTable);
        item->setText(secondsToTimeFormat(currentTime));
        emit currentTimeEdited(currentTime);
        emit dataChanged();
    });
    
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
    updateValue(ROW_CURRENT_TIME, secondsToTimeFormat(time));
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

void GeographicalInfo::updateGeoDataFromPositions(float currentTime,
                                                  float ownshipX, float ownshipY,
                                                  float targetX, float targetY,
                                                  float torpedoX, float torpedoY)
{
    GeoData data;
    data.currentTime = currentTime;
    data.ownshipTorpedoBearing = calculateBearing(ownshipX, ownshipY, torpedoX, torpedoY);
    data.ownshipTorpedoRange = calculateRange(ownshipX, ownshipY, torpedoX, torpedoY);
    data.ownshipTargetBearing = calculateBearing(ownshipX, ownshipY, targetX, targetY);
    data.ownshipTargetRange = calculateRange(ownshipX, ownshipY, targetX, targetY);
    data.torpedoTargetBearing = calculateBearing(torpedoX, torpedoY, targetX, targetY);
    data.torpedoTargetRange = calculateRange(torpedoX, torpedoY, targetX, targetY);
    updateGeoData(data);
}

void GeographicalInfo::updateGeoDataFromSample(float currentTime,
                                               float targetX, float targetY,
                                               float torpedoX, float torpedoY,
                                               float ownshipX, float ownshipY)
{
    updateGeoDataFromPositions(currentTime,
                               ownshipX, ownshipY,
                               targetX, targetY,
                               torpedoX, torpedoY);
}

void GeographicalInfo::clearData()
{
    // Show deterministic dummy values until real trajectory data is loaded.
    GeoData dummy;
    dummy.currentTime = 0.0f;
    dummy.ownshipTorpedoBearing = 186.0f;
    dummy.ownshipTorpedoRange = 1000.0f;
    dummy.ownshipTargetBearing = 194.0f;
    dummy.ownshipTargetRange = 1460.0f;
    dummy.torpedoTargetBearing = 202.0f;
    dummy.torpedoTargetRange = 1320.0f;
    updateGeoData(dummy);
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

QString GeographicalInfo::secondsToTimeFormat(float seconds)
{
    int totalSeconds = static_cast<int>(seconds);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;
    
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(secs, 2, 10, QLatin1Char('0'));
}

float GeographicalInfo::timeFormatToSeconds(const QString& timeStr)
{
    bool okSeconds = false;
    const float rawSeconds = timeStr.toFloat(&okSeconds);
    if (okSeconds && rawSeconds >= 0.0f) {
        return rawSeconds;
    }

    // Parse hh:mm:ss format
    QStringList parts = timeStr.split(':');
    if (parts.length() != 3) {
        return -1.0f;
    }
    
    bool okHours = false;
    bool okMinutes = false;
    bool okSecs = false;
    int hours = parts[0].toInt(&okHours);
    int minutes = parts[1].toInt(&okMinutes);
    int seconds = parts[2].toInt(&okSecs);
    
    if (!okHours || !okMinutes || !okSecs || hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60) {
        return -1.0f;
    }
    
    return static_cast<float>(hours * 3600 + minutes * 60 + seconds);
}
