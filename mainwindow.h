#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QString>
#include <QLabel>
#include <QTableWidget>
#include <QColumnView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include "eventtablewidget.h"
#include "trajectoryview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFileOpen();
    void onFileClose();
    void onFileParser();
    void onViewParameter();
    void onViewEvent();
    void onViewTrajectory();
    void onPrintParameterView();
    void onPrintEventView();
    void onPrintTrajectoryView();
    void onEventRowClicked(int row);

private:
    // Central widget and main layout
    QWidget *centralWidget;
    QStackedWidget *viewStack;
    
    // Menu bar and menus
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuView;
    QMenu *menuPrint;
    QMenu *menuParser;
    
    // Menu actions
    QAction *actionFileOpen;
    QAction *actionFileClose;
    QAction *actionFileParser;
    QAction *actionFileExit;
    QAction *actionViewParameter;
    QAction *actionViewEvent;
    QAction *actionViewTrajectory;
    QAction *actionPrintParameterView;
    QAction *actionPrintEventView;
    QAction *actionPrintTrajectoryView;
    
    // Status bar
    QStatusBar *statusBar;
    
    // Parameter view widgets
    QLabel *geoInfoLabel;
    QTableWidget *geoInfoTable;
    QColumnView *tubeSelect;
    QColumnView *trajectoryPlot;
    QColumnView *tablePlot;
    
    // Views
    EventTableWidget *eventView;
    TrajectoryView *trajectoryView;
    
    QString currentDatFilePath;
    
    void setupUI();
    void setupMenuBar();
    void setupViewStack();
    void showParserDialog(const QString& initialFilePath = QString(), bool autoStart = false);
    void switchToParameterView();
    void switchToEventView();
    void switchToTrajectoryView();
};
#endif // MAINWINDOW_H
