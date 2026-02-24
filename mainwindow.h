#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    Ui::MainWindow *ui;
    EventTableWidget *eventView;
    TrajectoryView *trajectoryView;
    QWidget *currentView;
    
    void showEventView();
    void showTrajectoryView();
    void hideEventView();
    void hideTrajectoryView();
};
#endif // MAINWINDOW_H
