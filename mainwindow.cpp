#include "mainwindow.h"
#include "datparser.h"
#include "eventtablewidget.h"
#include "trajectoryview.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QAction>
#include <QStackedWidget>
#include <QScreen>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , viewStack(nullptr)
    , menuBar(nullptr)
    , geoInfoLabel(nullptr)
    , geoInfoTable(nullptr)
    , tubeSelect(nullptr)
    , trajectoryPlot(nullptr)
    , tablePlot(nullptr)
    , eventView(nullptr)
    , trajectoryView(nullptr)
{
    // Setup the window
    setWindowTitle("MainWindow");
    
    // Create main UI
    setupUI();
    
    // Setup view stack for content switching
    setupViewStack();
    
    // Add some sample events for demonstration
    eventView->addEvent("Sensor1", "10:30:45.123", "10:30:50.456", "10:31:00.789", "CONTACT", "Target Detected");
    eventView->addEvent("Sensor2", "10:31:00.234", "10:31:05.567", "10:31:15.890", "ENGAGEMENT", "Firing Solution Computed");
    eventView->addEvent("Sensor1", "10:31:15.345", "10:31:20.678", "10:31:30.001", "DETONATION", "Impact Detected");
    
    // Connect menu actions
    connect(actionFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(actionFileClose, &QAction::triggered, this, &MainWindow::onFileClose);
    connect(actionFileParser, &QAction::triggered, this, &MainWindow::onFileParser);
    connect(actionFileExit, &QAction::triggered, qApp, &QApplication::quit);
    
    // Connect View menu actions
    connect(actionViewParameter, &QAction::triggered, this, &MainWindow::onViewParameter);
    connect(actionViewEvent, &QAction::triggered, this, &MainWindow::onViewEvent);
    connect(actionViewTrajectory, &QAction::triggered, this, &MainWindow::onViewTrajectory);
    
    // Connect Print menu actions
    connect(actionPrintParameterView, &QAction::triggered, this, &MainWindow::onPrintParameterView);
    connect(actionPrintEventView, &QAction::triggered, this, &MainWindow::onPrintEventView);
    connect(actionPrintTrajectoryView, &QAction::triggered, this, &MainWindow::onPrintTrajectoryView);
    
    // Connect event table row click to trajectory view
    connect(eventView, &EventTableWidget::rowClicked, this, &MainWindow::onEventRowClicked);
    
    // Set full screen mode
    showFullScreen();
    
    // Show parameter view by default
    switchToParameterView();
}

MainWindow::~MainWindow()
{
    // Qt handles cleanup of child widgets automatically
}

void MainWindow::setupUI()
{
    // Setup menu bar
    setupMenuBar();
    
    // Setup status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
}

void MainWindow::setupMenuBar()
{
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File Menu
    menuFile = menuBar->addMenu("File");
    
    actionFileOpen = menuFile->addAction("Open");
    actionFileOpen->setShortcut(Qt::CTRL | Qt::Key_O);
    
    actionFileClose = menuFile->addAction("Close");
    actionFileClose->setShortcut(Qt::CTRL | Qt::Key_W);
    
    menuFile->addSeparator();
    
    actionFileExit = menuFile->addAction("Exit");
    actionFileExit->setShortcut(Qt::CTRL | Qt::Key_Q);
    
    // View Menu
    menuView = menuBar->addMenu("View");
    
    actionViewParameter = menuView->addAction("Parameter");
    actionViewEvent = menuView->addAction("Event");
    actionViewTrajectory = menuView->addAction("Trajectory");
    
    // Print Menu
    menuPrint = menuBar->addMenu("Print");
    
    actionPrintParameterView = menuPrint->addAction("Parameter View");
    actionPrintEventView = menuPrint->addAction("Event View");
    actionPrintTrajectoryView = menuPrint->addAction("Trajectory View");
    
    // Parser Menu
    menuParser = menuBar->addMenu("Parser");
    
    actionFileParser = menuParser->addAction("Parser");
    actionFileParser->setShortcut(Qt::CTRL | Qt::Key_P);
}

void MainWindow::setupViewStack()
{
    // Create the stacked widget to hold the different views
    viewStack = new QStackedWidget(this);
    
    // Create parameter view widget
    QWidget *parameterViewWidget = new QWidget();
    QHBoxLayout *paramLayout = new QHBoxLayout(parameterViewWidget);
    paramLayout->setContentsMargins(0, 0, 0, 0);
    paramLayout->setSpacing(0);
    
    // Left panel with geographical information
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    
    // Geo Info Label
    geoInfoLabel = new QLabel("Geographical information");
    geoInfoLabel->setMinimumHeight(30);
    geoInfoLabel->setMaximumWidth(400);
    geoInfoLabel->setMaximumHeight(30);
    geoInfoLabel->setStyleSheet("background-color: rgb(85, 87, 83); color: white; font-weight: bold; padding-left: 5px;");
    geoInfoLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(geoInfoLabel);
    
    // Geo Info Table
    geoInfoTable = new QTableWidget();
    geoInfoTable->setColumnCount(2);
    geoInfoTable->setRowCount(7);
    geoInfoTable->setMinimumWidth(280);
    geoInfoTable->setMaximumWidth(400);
    geoInfoTable->setStyleSheet(
        "QTableWidget { \n"
        "  gridline-color: rgba(255, 255, 255, 0);\n"
        "  border: 1px solid rgba(0, 17, 255, 0.47);\n"
        "}\n"
        "QTableWidget::item {\n"
        "  padding: 5px;\n"
        "  border: 1px solid rgb(3, 95, 180);\n"
        "  color: rgb(255, 255, 255);\n"
        "}"
    );
    geoInfoTable->setFrameShape(QFrame::NoFrame);
    geoInfoTable->setFrameShadow(QFrame::Raised);
    geoInfoTable->setLineWidth(0);
    geoInfoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    geoInfoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    geoInfoTable->setAlternatingRowColors(true);
    geoInfoTable->setShowGrid(true);
    geoInfoTable->setGridStyle(Qt::SolidLine);
    geoInfoTable->horizontalHeader()->setVisible(false);
    geoInfoTable->verticalHeader()->setVisible(false);
    
    // Setup table items
    QStringList geoHeaders = {
        "Current Selected Time",
        "Ownship / Torpedo Bearing",
        "Ownship / Torpedo Range",
        "Ownship / Target Bearing",
        "Ownship / Target Range",
        "Torpedo / Target Bearing",
        "Torpedo / Target Range"
    };
    
    for (int i = 0; i < 7; ++i) {
        QTableWidgetItem *item = new QTableWidgetItem(geoHeaders[i]);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        geoInfoTable->setItem(i, 0, item);
    }
    
    leftLayout->addWidget(geoInfoTable);
    leftLayout->addStretch();
    
    // Right panel with plots
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    
    // Trajectory plot layout
    QHBoxLayout *trajectoryLayout = new QHBoxLayout();
    
    // Tube select
    tubeSelect = new QColumnView();
    tubeSelect->setMaximumWidth(150);
    tubeSelect->setStyleSheet("border: 1px solid gray;");
    trajectoryLayout->addWidget(tubeSelect);
    
    // Trajectory plot
    trajectoryPlot = new QColumnView();
    trajectoryPlot->setStyleSheet("border: 1px solid gray;");
    trajectoryLayout->addWidget(trajectoryPlot);
    
    rightLayout->addLayout(trajectoryLayout, 1);
    
    // Table plot
    tablePlot = new QColumnView();
    tablePlot->setStyleSheet("border: 1px solid gray;");
    rightLayout->addWidget(tablePlot, 1);
    
    paramLayout->addWidget(leftPanel);
    paramLayout->addWidget(rightPanel, 1);
    
    // Add parameter view to stacked widget
    viewStack->addWidget(parameterViewWidget);    // Index 0: Parameter view
    
    // Create and add event view
    eventView = new EventTableWidget(this);
    viewStack->addWidget(eventView);            // Index 1: Event view
    
    // Create and add trajectory view
    trajectoryView = new TrajectoryView(this);
    viewStack->addWidget(trajectoryView);       // Index 2: Trajectory view
    
    // Set the stacked widget as the central widget
    setCentralWidget(viewStack);
}


void MainWindow::onFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, 
        tr("Open File"), "", 
        tr("Data Files (*.dat *.txt);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        qDebug() << "Opening file:" << fileName;

        DatParser parser;
        if (!parser.parseFile(fileName.toStdString())) {
            QMessageBox::critical(this, tr("Parser"), tr("Failed to parse selected file."));
            return;
        }

        currentDatFilePath = fileName;
        QMessageBox::information(this, tr("Parser"), tr("Parsing completed."));
    }
}

void MainWindow::onFileClose()
{
    qDebug() << "Closing file";
    QMessageBox::information(this, tr("File Closed"), 
        tr("Current file has been closed."));
}

void MainWindow::onFileParser()
{
    if (!currentDatFilePath.trimmed().isEmpty()) {
        showParserDialog(currentDatFilePath, true);
        return;
    }

    showParserDialog();
}

void MainWindow::showParserDialog(const QString& initialFilePath, bool autoStart)
{
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Parser"));
    dialog.resize(1100, 700);
    dialog.setStyleSheet("border: 1px solid gray;");

    auto *layout = new QVBoxLayout(&dialog);
    auto *controlsLayout = new QHBoxLayout();

    auto *filePathEdit = new QLineEdit(&dialog);
    filePathEdit->setReadOnly(true);
    filePathEdit->setPlaceholderText(tr("Select a .dat file"));
    filePathEdit->setStyleSheet("border: 1px solid gray;");

    auto *openButton = new QPushButton(tr("Open"), &dialog);
    openButton->setStyleSheet("border: 1px solid gray;");
    auto *startButton = new QPushButton(tr("Start"), &dialog);
    startButton->setStyleSheet("border: 1px solid gray;");

    controlsLayout->addWidget(filePathEdit, 1);
    controlsLayout->addWidget(openButton);
    controlsLayout->addWidget(startButton);

    auto *tabs = new QTabWidget(&dialog);

    auto createTable = [&dialog](const QStringList &headers) {
        auto *table = new QTableWidget(&dialog);
        table->setColumnCount(headers.size());
        table->setHorizontalHeaderLabels(headers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setStyleSheet(
            "QTableWidget { border: 2px solid gray; }\n"
            "QTableWidget::item { border: 1px solid gray; }"
        );
        return table;
    };

    auto *ownTable = createTable({tr("Time"), tr("Course"), tr("Speed"), tr("Depth"), tr("X"), tr("Y")});
    auto *targetTable = createTable({tr("Time"), tr("Bearing"), tr("Range"), tr("Course"), tr("Speed"), tr("Depth"), tr("F1"), tr("F2"), tr("F3"), tr("F4"), tr("F5"), tr("F6"), tr("F7"), tr("F8"), tr("X"), tr("Y")});
    auto *torpedoTable = createTable({tr("Time"), tr("Bearing"), tr("Range"), tr("Course"), tr("Speed"), tr("Depth"), tr("Sonar"), tr("Wake"), tr("X"), tr("Y")});
    auto *tubeTable = createTable({tr("Time"), tr("Bearing"), tr("Range"), tr("Course"), tr("Speed"), tr("Depth"), tr("Sonar"), tr("Wake"), tr("X"), tr("Y")});
    auto *eventTable = createTable({tr("Time"), tr("Sender"), tr("Type"), tr("Name")});
    auto *sonarTable = createTable({tr("Time"), tr("Weapon"), tr("BeamX"), tr("BeamY"), tr("Intensity")});

    tabs->addTab(ownTable, tr("Own"));
    tabs->addTab(targetTable, tr("Target"));
    tabs->addTab(torpedoTable, tr("Torpedo"));
    tabs->addTab(tubeTable, tr("Tube"));
    tabs->addTab(eventTable, tr("Event"));
    tabs->addTab(sonarTable, tr("Sonar"));

    layout->addLayout(controlsLayout);
    layout->addWidget(tabs, 1);

    auto runParsing = [filePathEdit, ownTable, targetTable, torpedoTable, tubeTable, eventTable, sonarTable, this]() -> bool {
        const QString filePath = filePathEdit->text().trimmed();
        if (filePath.isEmpty()) {
            QMessageBox::warning(this, tr("Parser"), tr("Please open a .dat file first."));
            return false;
        }

        DatParser parser;
        if (!parser.parseFile(filePath.toStdString())) {
            QMessageBox::critical(this, tr("Parser"), tr("Failed to parse selected file."));
            return false;
        }

        const auto &ownRecords = parser.getOwnRecords();
        ownTable->setRowCount(static_cast<int>(ownRecords.size()));
        for (int row = 0; row < static_cast<int>(ownRecords.size()); ++row) {
            const OwnRecord &record = ownRecords[static_cast<size_t>(row)];
            ownTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            ownTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.course, 'f', 3)));
            ownTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.speed, 'f', 3)));
            ownTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.depth, 'f', 3)));
            ownTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.x, 'f', 3)));
            ownTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.y, 'f', 3)));
        }

        const auto &targetRecords = parser.getTargetRecords();
        targetTable->setRowCount(static_cast<int>(targetRecords.size()));
        for (int row = 0; row < static_cast<int>(targetRecords.size()); ++row) {
            const TargetRecord &record = targetRecords[static_cast<size_t>(row)];
            targetTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            targetTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.bearing, 'f', 3)));
            targetTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.range, 'f', 3)));
            targetTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.course, 'f', 3)));
            targetTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.speed, 'f', 3)));
            targetTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.depth, 'f', 3)));
            targetTable->setItem(row, 6, new QTableWidgetItem(QString::number(record.f1, 'f', 3)));
            targetTable->setItem(row, 7, new QTableWidgetItem(QString::number(record.f2, 'f', 3)));
            targetTable->setItem(row, 8, new QTableWidgetItem(QString::number(record.f3, 'f', 3)));
            targetTable->setItem(row, 9, new QTableWidgetItem(QString::number(record.f4, 'f', 3)));
            targetTable->setItem(row, 10, new QTableWidgetItem(QString::number(record.f5, 'f', 3)));
            targetTable->setItem(row, 11, new QTableWidgetItem(QString::number(record.f6, 'f', 3)));
            targetTable->setItem(row, 12, new QTableWidgetItem(QString::number(record.f7, 'f', 3)));
            targetTable->setItem(row, 13, new QTableWidgetItem(QString::number(record.f8, 'f', 3)));
            targetTable->setItem(row, 14, new QTableWidgetItem(QString::number(record.x, 'f', 3)));
            targetTable->setItem(row, 15, new QTableWidgetItem(QString::number(record.y, 'f', 3)));
        }

        const auto &torpedoRecords = parser.getTorpedoRecords();
        torpedoTable->setRowCount(static_cast<int>(torpedoRecords.size()));
        for (int row = 0; row < static_cast<int>(torpedoRecords.size()); ++row) {
            const TorpedoRecord &record = torpedoRecords[static_cast<size_t>(row)];
            torpedoTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            torpedoTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.bearing, 'f', 3)));
            torpedoTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.range, 'f', 3)));
            torpedoTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.course, 'f', 3)));
            torpedoTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.speed, 'f', 3)));
            torpedoTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.depth, 'f', 3)));
            torpedoTable->setItem(row, 6, new QTableWidgetItem(QString::number(record.sonar, 'f', 3)));
            torpedoTable->setItem(row, 7, new QTableWidgetItem(QString::number(record.wake, 'f', 3)));
            torpedoTable->setItem(row, 8, new QTableWidgetItem(QString::number(record.x, 'f', 3)));
            torpedoTable->setItem(row, 9, new QTableWidgetItem(QString::number(record.y, 'f', 3)));
        }

        const auto &tubeRecords = parser.getTubeRecords();
        tubeTable->setRowCount(static_cast<int>(tubeRecords.size()));
        for (int row = 0; row < static_cast<int>(tubeRecords.size()); ++row) {
            const TubeRecord &record = tubeRecords[static_cast<size_t>(row)];
            tubeTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            tubeTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.bearing, 'f', 3)));
            tubeTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.range, 'f', 3)));
            tubeTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.course, 'f', 3)));
            tubeTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.speed, 'f', 3)));
            tubeTable->setItem(row, 5, new QTableWidgetItem(QString::number(record.depth, 'f', 3)));
            tubeTable->setItem(row, 6, new QTableWidgetItem(QString::number(record.sonar, 'f', 3)));
            tubeTable->setItem(row, 7, new QTableWidgetItem(QString::number(record.wake, 'f', 3)));
            tubeTable->setItem(row, 8, new QTableWidgetItem(QString::number(record.x, 'f', 3)));
            tubeTable->setItem(row, 9, new QTableWidgetItem(QString::number(record.y, 'f', 3)));
        }

        const auto &eventRecords = parser.getEventRecords();
        eventTable->setRowCount(static_cast<int>(eventRecords.size()));
        for (int row = 0; row < static_cast<int>(eventRecords.size()); ++row) {
            const EventRecord &record = eventRecords[static_cast<size_t>(row)];
            eventTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            eventTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.sender)));
            eventTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.type)));
            eventTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(record.name)));
        }

        const auto &sonarRecords = parser.getSonarRecords();
        sonarTable->setRowCount(static_cast<int>(sonarRecords.size()));
        for (int row = 0; row < static_cast<int>(sonarRecords.size()); ++row) {
            const SonarRecord &record = sonarRecords[static_cast<size_t>(row)];
            sonarTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.time, 'f', 3)));
            sonarTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.weapon)));
            sonarTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.beamX, 'f', 3)));
            sonarTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.beamY, 'f', 3)));
            sonarTable->setItem(row, 4, new QTableWidgetItem(QString::number(record.intensity, 'f', 3)));
        }

        QMessageBox::information(this, tr("Parser"), tr("Parsing completed."));
        return true;
    };

    connect(openButton, &QPushButton::clicked, &dialog, [this, filePathEdit, runParsing]() {
        const QString fileName = QFileDialog::getOpenFileName(
            this,
            tr("Open .dat File"),
            QString(),
            tr("DAT Files (*.dat);;All Files (*)"));

        if (!fileName.isEmpty()) {
            filePathEdit->setText(fileName);
            runParsing();
        }
    });

    connect(startButton, &QPushButton::clicked, &dialog, [runParsing]() {
        runParsing();
    });

    if (!initialFilePath.trimmed().isEmpty()) {
        filePathEdit->setText(initialFilePath.trimmed());
    }

    if (autoStart && !filePathEdit->text().isEmpty()) {
        runParsing();
    }

    dialog.exec();
}

void MainWindow::onViewParameter()
{
    qDebug() << "Switching to Parameter view";
    switchToParameterView();
    QMessageBox::information(this, tr("View Changed"), 
        tr("Switched to Parameter view."));
}

void MainWindow::onViewEvent()
{
    qDebug() << "Switching to Event view";
    switchToEventView();
}

void MainWindow::onViewTrajectory()
{
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}

void MainWindow::onPrintParameterView()
{
    qDebug() << "Printing Parameter View";
    QMessageBox::information(this, tr("Print"), 
        tr("Parameter View sent to printer."));
}

void MainWindow::onPrintEventView()
{
    qDebug() << "Printing Event View";
    QMessageBox::information(this, tr("Print"), 
        tr("Event View sent to printer."));
}

void MainWindow::onPrintTrajectoryView()
{
    qDebug() << "Printing Trajectory View";
    QMessageBox::information(this, tr("Print"), 
        tr("Trajectory View sent to printer."));
}

void MainWindow::switchToParameterView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(0);
        qDebug() << "Switched to Parameter view (index 0)";
    }
}

void MainWindow::switchToEventView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(1);
        qDebug() << "Switched to Event view (index 1)";
    }
}

void MainWindow::switchToTrajectoryView()
{
    if (viewStack) {
        viewStack->setCurrentIndex(2);
        qDebug() << "Switched to Trajectory view (index 2)";
    }
}

void MainWindow::onEventRowClicked(int row)
{
    qDebug() << "Event row clicked:" << row;
    qDebug() << "Switching to Trajectory view";
    switchToTrajectoryView();
}
