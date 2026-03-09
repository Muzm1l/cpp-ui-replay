#ifndef DEMONGRAPH_H
#define DEMONGRAPH_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QVBoxLayout>

/**
 * @brief Widget for displaying DEMON (Detection of Envelope Modulation on Noise) analysis
 * 
 * Displays DEMON analysis data in a graphical view.
 */
class DemonGraph : public QWidget
{
    Q_OBJECT

public:
    explicit DemonGraph(QWidget *parent = nullptr);
    ~DemonGraph();
    
    void clearData();  // Placeholder for compatibility

private:
    QVBoxLayout *mainLayout;
    QLabel *headerLabel;
    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    
    void setupUI();
};

#endif // DEMONGRAPH_H
