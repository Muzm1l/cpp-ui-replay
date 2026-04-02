#ifndef GEOGRAPHICALINFO_H
#define GEOGRAPHICALINFO_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief Reusable widget for displaying geographical information
 * 
 * This widget displays key geographical data such as bearing and range
 * between ownship, torpedo, and target.
 */
class GeographicalInfo : public QWidget
{
    Q_OBJECT

public:
    explicit GeographicalInfo(QWidget *parent = nullptr);
    ~GeographicalInfo();
    
    // Update methods for geographical data
    void setCurrentTime(float time);
    void setOwnshipTorpedoBearing(float bearing);
    void setOwnshipTorpedoRange(float range);
    void setOwnshipTargetBearing(float bearing);
    void setOwnshipTargetRange(float range);
    void setTorpedoTargetBearing(float bearing);
    void setTorpedoTargetRange(float range);
    
    // Batch update method
    struct GeoData {
        float currentTime;
        float ownshipTorpedoBearing;
        float ownshipTorpedoRange;
        float ownshipTargetBearing;
        float ownshipTargetRange;
        float torpedoTargetBearing;
        float torpedoTargetRange;
        
        GeoData() : currentTime(0), ownshipTorpedoBearing(0), ownshipTorpedoRange(0),
                    ownshipTargetBearing(0), ownshipTargetRange(0),
                    torpedoTargetBearing(0), torpedoTargetRange(0) {}
    };
    
    void updateGeoData(const GeoData& data);
    void updateGeoDataFromPositions(float currentTime,
                                    float ownshipX, float ownshipY,
                                    float targetX, float targetY,
                                    float torpedoX, float torpedoY);
    void updateGeoDataFromSample(float currentTime,
                                 float targetX, float targetY,
                                 float torpedoX, float torpedoY,
                                 float ownshipX = 0.0f, float ownshipY = 0.0f);
    void clearData();
    
    // Customization options
    void setHeaderVisible(bool visible);
    void setHeaderText(const QString& text);
    void setFixedWidthMode(bool enabled, int width = 400);

signals:
    void dataChanged();

private:
    QVBoxLayout *mainLayout;
    QLabel *headerLabel;
    QTableWidget *geoTable;
    
    void setupUI();
    void setupTable();
    void updateValue(int row, const QString& value);
    
    enum GeoRow {
        ROW_CURRENT_TIME = 0,
        ROW_OWNSHIP_TORPEDO_BEARING = 1,
        ROW_OWNSHIP_TORPEDO_RANGE = 2,
        ROW_OWNSHIP_TARGET_BEARING = 3,
        ROW_OWNSHIP_TARGET_RANGE = 4,
        ROW_TORPEDO_TARGET_BEARING = 5,
        ROW_TORPEDO_TARGET_RANGE = 6
    };
};

#endif // GEOGRAPHICALINFO_H
