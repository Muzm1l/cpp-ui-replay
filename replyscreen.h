#ifndef REPLYSCREEN_H
#define REPLYSCREEN_H

#include <QWidget>

class QTextEdit;
class QPushButton;

class ReplyScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ReplyScreen(QWidget *parent = nullptr);
    ~ReplyScreen();

private slots:
    void on_closeButton_clicked();

private:
    QTextEdit *replyTextEdit;
    QPushButton *closeButton;
};

#endif // REPLYSCREEN_H
