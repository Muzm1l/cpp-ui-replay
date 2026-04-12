#include "parametrview.h"
#include <QHeaderView>
#include <QResizeEvent>
#include <QTime>
#include <QPen>
#include <QBrush>
#include <QtMath>

ParametrView::ParametrView(QWidget *parent)
    : QWidget(parent), parameterTable(nullptr), layout(nullptr), leftColumnLayout(nullptr), rightLayout(nullptr), acousticPanorama(nullptr), demonGraph(nullptr),
    sliderAT(nullptr), sliderAD(nullptr), sliderAY(nullptr), sliderAX(nullptr), scaleView(nullptr), topRightGraphicsView(nullptr), topRightScene(nullptr), rightGraphicsView(nullptr), rightScene(nullptr),
    topCursorLine(nullptr), bottomCursorLine(nullptr), topZoomRect(nullptr), bottomZoomRect(nullptr),
    engagementStartTime(), engagementEndTime(), currentCursorTime(), currentZoomStartTime(), currentZoomEndTime(),
    hasCursorTime(false), hasZoomRange(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // Left panel - matching TrajectoryView sizing
    QWidget *leftPanel = new QWidget();
    leftColumnLayout = new QVBoxLayout(leftPanel);
    leftColumnLayout->setContentsMargins(0, 0, 0, 0);
    leftColumnLayout->setSpacing(5);
    leftPanel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    leftPanel->setFixedWidth(400);
    
    // Acoustic Panorama Widget
    acousticPanorama = new AcousticPanorama();
    leftColumnLayout->addWidget(acousticPanorama);

    // DEMON Widget
    demonGraph = new DemonGraph();
    leftColumnLayout->addWidget(demonGraph);

    // Geographical Info Widget
    geographicalInfo = new GeographicalInfo();
    leftColumnLayout->addWidget(geographicalInfo);

    // Parameter Info Tree (box tree view)
    parameterInfoTree = new ParameterInfoTree();
    leftColumnLayout->addWidget(parameterInfoTree);
    // // Vertical spacer
    // leftColumnLayout->addStretch();
    
    // Right panel
    QWidget *rightPanel = new QWidget();
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);
    
    setupRightLayout();
    
    // Add panels to main layout
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel, 1);
}

ParametrView::~ParametrView()
{
}

void ParametrView::setupTable()
{
    parameterTable->setColumnCount(2);
    parameterTable->setHorizontalHeaderLabels(QStringList() << "Parameter Name" << "Value");

    parameterTable->verticalHeader()->setVisible(false);
    parameterTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    parameterTable->horizontalHeader()->setStretchLastSection(true);

    parameterTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    parameterTable->setSelectionMode(QAbstractItemView::SingleSelection);
    parameterTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    parameterTable->setAlternatingRowColors(true);

    parameterTable->setMinimumSize(500, 400);
}

void ParametrView::setupRightLayout()
{
    constexpr int kTopLeftPanelWidth = 400;

    QWidget *topHalfWidget = new QWidget();
    QVBoxLayout *topHalfLayout = new QVBoxLayout(topHalfWidget);
    topHalfLayout->setContentsMargins(0, 0, 0, 0);
    topHalfLayout->setSpacing(5);

    // Top section - split into 2 parts horizontally
    QWidget *topSectionWidget = new QWidget();
    QHBoxLayout *topSectionLayout = new QHBoxLayout(topSectionWidget);
    topSectionLayout->setContentsMargins(0, 0, 0, 0);
    topSectionLayout->setSpacing(5);
    
    // Left part with 4 sliders (AT, AD, AY, AX)
    QWidget *topLeftWidget = new QWidget();
    topLeftWidget->setObjectName("parameterTopLeftWidget");
    topLeftWidget->setFixedWidth(kTopLeftPanelWidth);
    QHBoxLayout *topLeftLayout = new QHBoxLayout(topLeftWidget);
    topLeftLayout->setContentsMargins(10, 10, 10, 10);
    topLeftLayout->setSpacing(20);
    
    sliderAT = new QSlider(Qt::Vertical, this);
    sliderAT->setRange(0, 100);
    sliderAT->setMinimumHeight(120);
    QVBoxLayout *atLayout = new QVBoxLayout();
    atLayout->addWidget(sliderAT);
    QLabel *atLabel = new QLabel("AT");
    atLabel->setAlignment(Qt::AlignCenter);
    atLayout->addWidget(atLabel);
    topLeftLayout->addLayout(atLayout);
    
    sliderAD = new QSlider(Qt::Vertical, this);
    sliderAD->setRange(0, 100);
    sliderAD->setMinimumHeight(120);
    QVBoxLayout *adLayout = new QVBoxLayout();
    adLayout->addWidget(sliderAD);
    QLabel *adLabel = new QLabel("AD");
    adLabel->setAlignment(Qt::AlignCenter);
    adLayout->addWidget(adLabel);
    topLeftLayout->addLayout(adLayout);
    
    sliderAY = new QSlider(Qt::Vertical, this);
    sliderAY->setRange(0, 100);
    sliderAY->setMinimumHeight(120);
    QVBoxLayout *ayLayout = new QVBoxLayout();
    ayLayout->addWidget(sliderAY);
    QLabel *ayLabel = new QLabel("AY");
    ayLabel->setAlignment(Qt::AlignCenter);
    ayLayout->addWidget(ayLabel);
    topLeftLayout->addLayout(ayLayout);
    
    sliderAX = new QSlider(Qt::Vertical, this);
    sliderAX->setRange(0, 100);
    sliderAX->setMinimumHeight(120);
    QVBoxLayout *axLayout = new QVBoxLayout();
    axLayout->addWidget(sliderAX);
    QLabel *axLabel = new QLabel("AX");
    axLabel->setAlignment(Qt::AlignCenter);
    axLayout->addWidget(axLabel);
    topLeftLayout->addLayout(axLayout);
    
    topLeftLayout->addStretch();
    topSectionLayout->addWidget(topLeftWidget);
    
    // Right part with graphics view
    QWidget *topRightWidget = new QWidget();
    topRightWidget->setObjectName("parameterTopRightWidget");
    QVBoxLayout *topRightLayout = new QVBoxLayout(topRightWidget);
    topRightLayout->setContentsMargins(0, 0, 0, 0);
    
    topRightScene = new QGraphicsScene(this);
    topRightGraphicsView = new QGraphicsView(topRightScene, this);
    topRightGraphicsView->setObjectName("parameterTopRightGraphicsView");
    topRightGraphicsView->setFrameShape(QFrame::NoFrame);
    topRightLayout->addWidget(topRightGraphicsView);
    
    topSectionLayout->addWidget(topRightWidget, 1);

    topHalfLayout->addWidget(topSectionWidget, 8);

    // Middle section with time scale view
    QWidget *scaleRowWidget = new QWidget();
    QHBoxLayout *scaleRowLayout = new QHBoxLayout(scaleRowWidget);
    scaleRowLayout->setContentsMargins(0, 0, 0, 0);
    scaleRowLayout->setSpacing(5);

    QWidget *scaleLeftSpacer = new QWidget();
    scaleLeftSpacer->setFixedWidth(kTopLeftPanelWidth);
    scaleLeftSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    scaleRowLayout->addWidget(scaleLeftSpacer);

    scaleView = new ScaleView(this);
    scaleView->setObjectName("parameterScaleView");
    scaleView->setMinimumHeight(0);
    engagementStartTime = QTime(9, 34, 15);
    engagementEndTime = QTime(9, 43, 39);
    scaleView->setTimeRange(engagementStartTime, engagementEndTime);
    scaleView->setMarkers(
        QVector<QTime>{
            QTime(9, 38, 35),
            QTime(9, 39, 15),
            QTime(9, 39, 26),
            QTime(9, 39, 49),
            QTime(9, 40, 26),
            QTime(9, 41, 5),
            QTime(9, 41, 15),
            QTime(9, 41, 40)
        },
        QStringList()
            << "09:38:35"
            << "09:39:15"
            << "09:39:26"
            << "09:39:49"
            << "09:40:26"
            << "09:41:05"
            << "09:41:15"
            << "09:41:40"
    );
    scaleView->setHighlightedTime(QTime(9, 39, 26));
    scaleView->setSelectionRange(QTime(9, 39, 58), QTime(9, 41, 5));
    scaleRowLayout->addWidget(scaleView, 1);

    topHalfLayout->addWidget(scaleRowWidget, 2);

    rightLayout->addWidget(topHalfWidget, 1);
    
    QWidget *bottomHalfWidget = new QWidget();
    QVBoxLayout *bottomHalfLayout = new QVBoxLayout(bottomHalfWidget);
    bottomHalfLayout->setContentsMargins(0, 0, 0, 0);
    bottomHalfLayout->setSpacing(5);

    rightScene = new QGraphicsScene(this);
    rightGraphicsView = new QGraphicsView(rightScene, this);
    rightGraphicsView->setObjectName("parameterRightGraphicsView");
    rightGraphicsView->setMinimumHeight(0);
    bottomHalfLayout->addWidget(rightGraphicsView);

    rightLayout->addWidget(bottomHalfWidget, 1);

    connect(scaleView, &ScaleView::highlightedTimeChanged, this, [this](const QTime &time) {
        currentCursorTime = time;
        hasCursorTime = time.isValid();
        syncScaleToGraphics();
    });

    connect(scaleView, &ScaleView::selectionRangeChanged, this, [this](const QTime &startTime, const QTime &endTime) {
        currentZoomStartTime = startTime;
        currentZoomEndTime = endTime;
        hasZoomRange = startTime.isValid() && endTime.isValid() && endTime > startTime;
        syncScaleToGraphics();
    });

    currentCursorTime = QTime(9, 39, 26);
    hasCursorTime = true;
    currentZoomStartTime = QTime(9, 39, 58);
    currentZoomEndTime = QTime(9, 41, 5);
    hasZoomRange = true;
    syncScaleToGraphics();
}

void ParametrView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    syncScaleToGraphics();
}

void ParametrView::ensureSceneRectMatchesView(QGraphicsView *view, QGraphicsScene *scene)
{
    if (!view || !scene || !view->viewport()) {
        return;
    }

    const QRect viewportRect = view->viewport()->rect();
    if (!viewportRect.isValid() || viewportRect.width() <= 0 || viewportRect.height() <= 0) {
        return;
    }

    scene->setSceneRect(QRectF(0, 0, viewportRect.width(), viewportRect.height()));
}

double ParametrView::timeRatio(const QTime &time) const
{
    if (!time.isValid() || !engagementStartTime.isValid() || !engagementEndTime.isValid()) {
        return 0.0;
    }

    const int totalSeconds = qMax(1, engagementStartTime.secsTo(engagementEndTime));
    const int offsetSeconds = qBound(0, engagementStartTime.secsTo(time), totalSeconds);
    return static_cast<double>(offsetSeconds) / static_cast<double>(totalSeconds);
}

void ParametrView::syncScaleToGraphics()
{
    ensureSceneRectMatchesView(topRightGraphicsView, topRightScene);
    ensureSceneRectMatchesView(rightGraphicsView, rightScene);

    if (!topRightScene || !rightScene) {
        return;
    }

    const QRectF topRect = topRightScene->sceneRect();
    const QRectF bottomRect = rightScene->sceneRect();
    if (!topRect.isValid() || !bottomRect.isValid()) {
        return;
    }

    const QPen zoomPen(QColor(40, 120, 210, 150));
    const QBrush zoomBrush(QColor(40, 120, 210, 35));
    const QPen cursorPen(QColor(40, 120, 210));

    if (hasZoomRange) {
        const double startRatio = timeRatio(currentZoomStartTime);
        const double endRatio = timeRatio(currentZoomEndTime);

        const QRectF topZoom(topRect.left() + (startRatio * topRect.width()),
                             topRect.top(),
                             qMax(1.0, (endRatio - startRatio) * topRect.width()),
                             topRect.height());
        const QRectF bottomZoom(bottomRect.left() + (startRatio * bottomRect.width()),
                                bottomRect.top(),
                                qMax(1.0, (endRatio - startRatio) * bottomRect.width()),
                                bottomRect.height());

        if (!topZoomRect) {
            topZoomRect = topRightScene->addRect(topZoom, zoomPen, zoomBrush);
            topZoomRect->setZValue(-1.0);
        } else {
            topZoomRect->setRect(topZoom);
            topZoomRect->setPen(zoomPen);
            topZoomRect->setBrush(zoomBrush);
        }

        if (!bottomZoomRect) {
            bottomZoomRect = rightScene->addRect(bottomZoom, zoomPen, zoomBrush);
            bottomZoomRect->setZValue(-1.0);
        } else {
            bottomZoomRect->setRect(bottomZoom);
            bottomZoomRect->setPen(zoomPen);
            bottomZoomRect->setBrush(zoomBrush);
        }
    } else {
        if (topZoomRect) {
            topRightScene->removeItem(topZoomRect);
            topZoomRect = nullptr;
        }
        if (bottomZoomRect) {
            rightScene->removeItem(bottomZoomRect);
            bottomZoomRect = nullptr;
        }
    }

    if (hasCursorTime) {
        const double cursorRatio = timeRatio(currentCursorTime);
        const double topX = topRect.left() + (cursorRatio * topRect.width());
        const double bottomX = bottomRect.left() + (cursorRatio * bottomRect.width());

        const QLineF topLine(topX, topRect.top(), topX, topRect.bottom());
        const QLineF bottomLine(bottomX, bottomRect.top(), bottomX, bottomRect.bottom());

        if (!topCursorLine) {
            topCursorLine = topRightScene->addLine(topLine, cursorPen);
            topCursorLine->setZValue(2.0);
        } else {
            topCursorLine->setLine(topLine);
            topCursorLine->setPen(cursorPen);
        }

        if (!bottomCursorLine) {
            bottomCursorLine = rightScene->addLine(bottomLine, cursorPen);
            bottomCursorLine->setZValue(2.0);
        } else {
            bottomCursorLine->setLine(bottomLine);
            bottomCursorLine->setPen(cursorPen);
        }
    } else {
        if (topCursorLine) {
            topRightScene->removeItem(topCursorLine);
            topCursorLine = nullptr;
        }
        if (bottomCursorLine) {
            rightScene->removeItem(bottomCursorLine);
            bottomCursorLine = nullptr;
        }
    }
}
