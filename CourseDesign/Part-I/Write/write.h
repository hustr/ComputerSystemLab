#ifndef WRITE_H
#define WRITE_H

#include <QWidget>
#include <thread>
#include "config.hpp"

namespace Ui {
class Write;
}

class Write : public QWidget
{
    Q_OBJECT

public:
    explicit Write(QString str, QWidget *parent = nullptr);
    ~Write();
signals:
    void log(QString str);

private:
    void write();
    QString file;
    int b_valid, b_empty;
    Block *b_mem;
    std::thread *t;

    Ui::Write *ui;
};

#endif // WRITE_H
