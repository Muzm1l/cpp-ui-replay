#ifndef PARSERDIALOG_H
#define PARSERDIALOG_H

#include <QDialog>
#include <QString>

class QWidget;

class ParserDialog
{
public:
    static void show(QWidget *parent, const QString& initialFilePath = QString(), bool autoStart = false);
};

#endif // PARSERDIALOG_H
