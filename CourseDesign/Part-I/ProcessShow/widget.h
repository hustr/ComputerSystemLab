#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <iostream>


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
    void read_output_ready();
    void copy_output_ready();
    void print_output_ready();

private:
    Ui::Widget *ui;
    // three process read file, move memory, write to file
    QProcess *read_process;
    QProcess *copy_process;
    QProcess *print_process;
    // semaphore
    int a_empty, a_valid, b_empty, b_valid;
    int shm_a, shm_b;

//public: slots:

};

#endif // WIDGET_H
