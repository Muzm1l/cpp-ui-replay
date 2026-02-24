#ifndef EVENTTABLEWIDGET_H
#define EVENTTABLEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>

class EventTableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventTableWidget(QWidget *parent = nullptr);
    ~EventTableWidget();
    
    void addEvent(const QString &sender, const QString &systemTime,
                  const QString &engagementTime, const QString &torpedoTime,
                  const QString &msgType, const QString &msgLabel);
    
    void clearEvents();
    QTableWidget* getTableWidget() { return eventTable; }

private:
    QTableWidget *eventTable;
    QVBoxLayout *layout;
    
    void setupTable();
};

#endif // EVENTTABLEWIDGET_H
