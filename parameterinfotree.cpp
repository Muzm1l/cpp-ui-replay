#include "parameterinfotree.h"
#include <QHeaderView>

ParameterInfoTree::ParameterInfoTree(QWidget *parent)
    : QTreeWidget(parent)
{
    setObjectName("parameterInfoTree");
    setHeaderLabel("Parameter Information");
    setMinimumHeight(120);
    header()->setMinimumHeight(30);
    header()->setMaximumHeight(30);
    header()->setDefaultAlignment(Qt::AlignCenter);
    // Example structure, replace with real parameter info as needed
    QTreeWidgetItem *cat1 = new QTreeWidgetItem(this, QStringList("Category 1"));
    cat1->addChild(new QTreeWidgetItem(cat1, QStringList("Parameter A")));
    cat1->addChild(new QTreeWidgetItem(cat1, QStringList("Parameter B")));
    QTreeWidgetItem *cat2 = new QTreeWidgetItem(this, QStringList("Category 2"));
    cat2->addChild(new QTreeWidgetItem(cat2, QStringList("Parameter X")));
    cat2->addChild(new QTreeWidgetItem(cat2, QStringList("Parameter Y")));
    expandAll();
}
