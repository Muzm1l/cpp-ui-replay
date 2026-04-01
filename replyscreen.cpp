#include "replyscreen.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

ReplyScreen::ReplyScreen(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("replyScreen");
    // Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(20);

    // Create the text edit area
    replyTextEdit = new QTextEdit(this);
    replyTextEdit->setObjectName("replyTextEdit");
    replyTextEdit->setPlaceholderText("Enter your reply...");
    layout->addWidget(replyTextEdit);

    // Create the close button
    closeButton = new QPushButton("Close", this);
    closeButton->setObjectName("dangerCloseButton");
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    // Connect the close button's clicked signal to the slot
    connect(closeButton, &QPushButton::clicked, this, &ReplyScreen::on_closeButton_clicked);

    // Make it full screen
    setWindowState(Qt::WindowFullScreen);
}

ReplyScreen::~ReplyScreen()
{
}

void ReplyScreen::on_closeButton_clicked()
{
    close(); // Close the widget
}
