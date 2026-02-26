#ifndef TRAJECTORYVIEW_H
#define TRAJECTORYVIEW_H

#include <QWidget>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class TrajectoryView;
}
QT_END_NAMESPACE

class TrajectoryView : public QWidget
{
    Q_OBJECT

public:
    explicit TrajectoryView(QWidget *parent = nullptr);
    ~TrajectoryView();

private:
    Ui::TrajectoryView *ui;
    double zoomLevel = 1.0;
    QGraphicsScene *trajectoryScene = nullptr;
    
    // Axes items to keep fixed
    QList<QGraphicsLineItem*> axisLines;
    QList<QGraphicsTextItem*> axisLabels;
    
    void initTrajectoryPlot();
    void populateDummyData();
    void updateAxesPosition();
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // TRAJECTORYVIEW_H
