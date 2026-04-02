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
#include <QActionGroup>
#include <QStatusBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <vector>
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
    void onTubeSelectionTriggered(QAction *action);

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
    QMenu *menuTubeSelection;
    
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
    QActionGroup *tubeSelectionActionGroup;
    
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
    int submarineTubeCount;
    std::size_t selectedTubeIndex;
    bool hasTubeSelection;
    std::vector<std::vector<std::size_t>> tubeTargetIndexPools;
    std::vector<QStringList> tubeTargetLabelPools;
    
    void setupUI();
    void setupMenuBar();
    void setupViewStack();
    void setupHomeWidget();
    void setupTubeSelectionMenu(int tubeCount);
    void setTubeSelectionMenuVisible(bool visible);
    void setupTubeTargetPools();
    void applyTargetsForSelectedTube();
    void initTrajectoryPlot();
    void populateDummyData();
    void switchToParameterView();
    void switchToEventView();
    void switchToTrajectoryView();
};
#endif // HOME_H
