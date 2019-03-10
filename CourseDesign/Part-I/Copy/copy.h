#ifndef COPY_H
#define COPY_H

#include <QWidget>
#include <thread>
#include <memory>
#include "config.hpp"

namespace Ui {
class Copy;
}

class Copy : public QWidget
{
    Q_OBJECT

public:
    explicit Copy(QWidget *parent = nullptr);
    ~Copy();
signals:
    void log(QString str);

private:
    void copy();


    int a_valid, a_empty, b_empty, b_valid;
    Block *a_mem, *b_mem;
    std::thread *t;
    Ui::Copy *ui;
};

#endif // COPY_H
