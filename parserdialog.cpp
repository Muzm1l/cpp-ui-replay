#include "parserdialog.h"
#include "parser.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>

void ParserDialog::show(QWidget *parent, const QString& initialFilePath, bool autoStart)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Parser"));
    dialog.resize(1100, 700);
    dialog.setStyleSheet("border: 1px solid gray;");

    auto *layout = new QVBoxLayout(&dialog);
    auto *controlsLayout = new QHBoxLayout();

    auto *filePathEdit = new QLineEdit(&dialog);
    filePathEdit->setReadOnly(true);
    filePathEdit->setPlaceholderText(QObject::tr("Select a .dat file"));
    filePathEdit->setStyleSheet("border: 1px solid gray;");

    auto *openButton = new QPushButton(QObject::tr("Open"), &dialog);
    openButton->setStyleSheet("border: 1px solid gray;");
    auto *startButton = new QPushButton(QObject::tr("Start"), &dialog);
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
        table->verticalHeader()->setVisible(false);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setStyleSheet(
            "QTableWidget { border: 2px solid gray; }\n"
            "QTableWidget::item { border: 1px solid gray; }"
        );
        return table;
    };

    auto *ownTable = createTable({QObject::tr("Time"), QObject::tr("Course"), QObject::tr("Speed"), QObject::tr("Depth"), QObject::tr("X"), QObject::tr("Y")});
    auto *targetTable = createTable({QObject::tr("Time"), QObject::tr("Bearing"), QObject::tr("Range"), QObject::tr("Course"), QObject::tr("Speed"), QObject::tr("Depth"), QObject::tr("F1"), QObject::tr("F2"), QObject::tr("F3"), QObject::tr("F4"), QObject::tr("F5"), QObject::tr("F6"), QObject::tr("F7"), QObject::tr("F8"), QObject::tr("X"), QObject::tr("Y")});
    auto *torpedoTable = createTable({QObject::tr("Time"), QObject::tr("Bearing"), QObject::tr("Range"), QObject::tr("Course"), QObject::tr("Speed"), QObject::tr("Depth"), QObject::tr("Sonar"), QObject::tr("Wake"), QObject::tr("X"), QObject::tr("Y")});
    auto *tubeTable = createTable({QObject::tr("Time"), QObject::tr("Bearing"), QObject::tr("Range"), QObject::tr("Course"), QObject::tr("Speed"), QObject::tr("Depth"), QObject::tr("Sonar"), QObject::tr("Wake"), QObject::tr("X"), QObject::tr("Y")});
    auto *eventTable = createTable({QObject::tr("Time"), QObject::tr("Sender"), QObject::tr("Type"), QObject::tr("Name")});
    auto *sonarTable = createTable({QObject::tr("Time"), QObject::tr("Weapon"), QObject::tr("BeamX"), QObject::tr("BeamY"), QObject::tr("Intensity")});

    tabs->addTab(ownTable, QObject::tr("Own"));
    tabs->addTab(targetTable, QObject::tr("Target"));
    tabs->addTab(torpedoTable, QObject::tr("Torpedo"));
    tabs->addTab(tubeTable, QObject::tr("Tube"));
    tabs->addTab(eventTable, QObject::tr("Event"));
    tabs->addTab(sonarTable, QObject::tr("Sonar"));

    layout->addLayout(controlsLayout);
    layout->addWidget(tabs, 1);

    auto runParsing = [filePathEdit, ownTable, targetTable, torpedoTable, tubeTable, eventTable, sonarTable, parent]() -> bool {
        const QString filePath = filePathEdit->text().trimmed();
        if (filePath.isEmpty()) {
            QMessageBox::warning(parent, QObject::tr("Parser"), QObject::tr("Please open a .dat file first."));
            return false;
        }

        DatParser parser;
        if (!parser.parseFile(filePath.toStdString())) {
            QMessageBox::critical(parent, QObject::tr("Parser"), QObject::tr("Failed to parse selected file."));
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
            eventTable->setItem(row, 0, new QTableWidgetItem(QString::number(record.sender)));
            eventTable->setItem(row, 1, new QTableWidgetItem(QString::number(record.systemTime, 'f', 3)));
            eventTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.engagementTime, 'f', 3)));
            eventTable->setItem(row, 3, new QTableWidgetItem(QString::number(record.torpedoTime, 'f', 3)));
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

        return true;
    };

    QObject::connect(openButton, &QPushButton::clicked, &dialog, [parent, filePathEdit, runParsing]() {
        const QString fileName = QFileDialog::getOpenFileName(
            parent,
            QObject::tr("Open .dat File"),
            QString(),
            QObject::tr("DAT Files (*.dat);;All Files (*)"));

        if (!fileName.isEmpty()) {
            filePathEdit->setText(fileName);
            runParsing();
        }
    });

    QObject::connect(startButton, &QPushButton::clicked, &dialog, [runParsing]() {
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
