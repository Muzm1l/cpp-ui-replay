#include "replyscreen.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

ReplyScreen::ReplyScreen(QWidget *parent)
    : QWidget(parent)
{
    // Set up the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(20);

    // Create the text edit area
    replyTextEdit = new QTextEdit(this);
    replyTextEdit->setPlaceholderText("Enter your reply...");
    replyTextEdit->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 5px; padding: 10px; font-size: 14px;");
    layout->addWidget(replyTextEdit);

    // Create the close button
    closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet("background-color: #d9534f; color: white; border: none; padding: 10px 20px; border-radius: 5px; font-size: 14px;");
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    // Connect the close button's clicked signal to the slot
    connect(closeButton, &QPushButton::clicked, this, &ReplyScreen::on_closeButton_clicked);

    // Set the background color and make it full screen
    setStyleSheet("background-color: #ffffff;");
    setWindowState(Qt::WindowFullScreen);
}

ReplyScreen::~ReplyScreen()
{
}

void ReplyScreen::on_closeButton_clicked()
{
    close(); // Close the widget
}
