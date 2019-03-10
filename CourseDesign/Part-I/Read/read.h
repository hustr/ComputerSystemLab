#ifndef READ_H
#define READ_H

#include <QWidget>
#include <thread>
#include <memory>
#include <iostream>
#include "config.hpp"

namespace Ui {
class Read;
}

class Read : public QWidget
{
    Q_OBJECT

public:
    explicit Read(QString file, QWidget *parent = nullptr);
    ~Read();
signals:
    void log(QString s);

private:
    void read_file();
    std::thread *t;

    int a_empty, a_valid, a_id;
    Block *mem;


    Ui::Read *ui;
    QString file;
};

#endif // READ_H
