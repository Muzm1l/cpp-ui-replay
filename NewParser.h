#ifndef NEWPARSER_H
#define NEWPARSER_H

#include <QFile>
#include <QDataStream>
#include <QVector>
#include <QString>
#include "parser.h" // Using existing structs

class NewParser
{
public:
    NewParser();
    bool parseFile(const QString& filePath);
    const QVector<TorpedoRecord>& getTorpedoRecords() const;
    const QVector<TargetRecord>& getTargetRecords() const;
    const QVector<SonarRecord>& getSonarRecords() const;
    const QVector<EventRecord>& getEventRecords() const;

private:
    void dispatchRecord(QDataStream& stream);

    QVector<TorpedoRecord> torpedoRecords;
    QVector<TargetRecord> targetRecords;
    QVector<SonarRecord> sonarRecords;
    QVector<EventRecord> eventRecords;
};

class ExporterModule
{
public:
    static bool exportToCsv(const QString& filePath, const QVector<TorpedoRecord>& records);
    static bool exportToCsv(const QString& filePath, const QVector<TargetRecord>& records);
    static bool exportToCsv(const QString& filePath, const QVector<SonarRecord>& records);
    static bool exportToCsv(const QString& filePath, const QVector<EventRecord>& records);
};

#endif // NEWPARSER_H
