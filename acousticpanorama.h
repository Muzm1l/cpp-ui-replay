#ifndef ACOUSTICPANORAMA_H
#define ACOUSTICPANORAMA_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>

/**
 * @brief Widget for displaying acoustic panorama
 * 
 * Displays acoustic data in a graphical view.
 */
class AcousticPanorama : public QWidget
{
    Q_OBJECT

public:
    explicit AcousticPanorama(QWidget *parent = nullptr);
    ~AcousticPanorama();

private:
    QVBoxLayout *mainLayout;
    QLabel *headerLabel;
    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    
    void setupUI();
};

#endif // ACOUSTICPANORAMA_H
