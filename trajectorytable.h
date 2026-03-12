#ifndef TRAJECTORYTABLE_H
#define TRAJECTORYTABLE_H

#include <QTableWidget>
#include <QPushButton>

class TrajectoryTable : public QTableWidget {
    Q_OBJECT
public:
    explicit TrajectoryTable(QWidget *parent = nullptr);
signals:
    void closeRequested();
private:
    QPushButton *closeButton = nullptr;
};

#endif // TRAJECTORYTABLE_H
