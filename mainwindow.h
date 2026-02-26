#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "eventtablewidget.h"
#include "trajectoryview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFileOpen();
    void onFileClose();
    void onFileExit();
    void onViewParameter();
    void onViewEvent();
    void onViewTrajectory();
    void onPrintParameterView();
    void onPrintEventView();
    void onPrintTrajectoryView();

private slots:
    void onEventRowClicked(int row);

private:
    Ui::MainWindow *ui;
    EventTableWidget *eventView;
    TrajectoryView *trajectoryView;
    QStackedWidget *viewStack;
    
    void setupViewStack();
    void switchToParameterView();
    void switchToEventView();
    void switchToTrajectoryView();
};
#endif // MAINWINDOW_H
