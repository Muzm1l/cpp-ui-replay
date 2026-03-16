#ifndef PARAMETERINFOTREE_H
#define PARAMETERINFOTREE_H

#include <QTreeWidget>
#include <QWidget>

class ParameterInfoTree : public QTreeWidget {
    Q_OBJECT
public:
    explicit ParameterInfoTree(QWidget *parent = nullptr);
};

#endif // PARAMETERINFOTREE_H
