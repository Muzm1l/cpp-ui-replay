#ifndef EVENTVIEW_H
#define EVENTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>

class EventView : public QWidget
{
    Q_OBJECT

public:
    explicit EventView(QWidget *parent = nullptr);
    ~EventView();
    
    void addEvent(const QString &sender, const QString &systemTime,
                  const QString &engagementTime, const QString &torpedoTime,
                  const QString &msgType, const QString &msgLabel);
    
    void clearEvents();
    QTableWidget* getTableWidget() { return eventTable; }

signals:
    void rowClicked(int row);

private:
    QTableWidget *eventTable;
    QVBoxLayout *layout;
    
    void setupTable();
};

#endif // EVENTVIEW_H
