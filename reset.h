#ifndef RESET_H
#define RESET_H

#include <QMainWindow>
#include <QTimer>
#include <QCommandLinkButton>
#include "branding.h"

namespace Ui {
class Reset;
}

class Reset : public QMainWindow
{
    Q_OBJECT

public:
    explicit Reset(QWidget *parent = 0);
    ~Reset();

    void showFullScreen();

private slots:
    void on_resetEverything_clicked();

    void on_backButton_clicked();

    void on_pages_currentChanged(int arg1);

private:
    Ui::Reset *ui;
};

#endif // RESET_H
