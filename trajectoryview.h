#ifndef TRAJECTORYVIEW_H
#define TRAJECTORYVIEW_H

#include <QWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();

private:
    QListWidget *tubeList;
    QWidget *trajectoryPlot;
    QTableWidget *trajectoryTable;
    
    void setupUI();
    void populateDummyData();
};

#endif // TRAJECTORYVIEW_H
