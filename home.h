#ifndef HOME_H
#define HOME_H

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
#include <QGraphicsScene>
#include <QGraphicsView>
#include "eventview.h"
#include "parametrview.h"
#include "trajectoryview.h"
#include "geographicalinfo.h"
#include "parserdialog.h"

class Home : public QMainWindow
{
    Q_OBJECT

public:
    Home(QWidget *parent = nullptr);
    ~Home();

private slots:
    void onFileOpen();
    void onFileClose();
    void onFileParser();
    void onGenerateSimulator();
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
    QAction *actionGenerateSimulator;
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
    
    // Views
    ParametrView *parameterView;
    EventView *eventView;
    TrajectoryView *trajectoryView;
    
    // Home page widgets (similar to TrajectoryView but without acoustic/demon)
    QWidget *homeWidget;
    QGraphicsView *trajectoryPlot;
    QColumnView *tubeSelect;
    QColumnView *tablePlot;
    GeographicalInfo *geoInfo;
    QGraphicsScene *trajectoryScene;
    
    QString currentDatFilePath;
    
    void setupUI();
    void setupMenuBar();
    void setupViewStack();
    void setupHomeWidget();
    void initTrajectoryPlot();
    void populateDummyData();
    void switchToParameterView();
    void switchToEventView();
    void switchToTrajectoryView();
};
#endif // HOME_H
