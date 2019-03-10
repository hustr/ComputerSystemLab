#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <QFileDialog>
#include <iostream>
#include <QDebug>
#include "config.hpp"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_start_btn_clicked();

    void on_src_btn_clicked();

    void on_dst_btn_clicked();

private:
    Ui::Widget *ui;
    // three process read file, move memory, write to file
    QProcess *read_process;
    QProcess *copy_process;
    QProcess *print_process;
    // semaphore
    int a_empty, a_valid, b_empty, b_valid;
    int shm_a, shm_b;
};

#endif // WIDGET_H
