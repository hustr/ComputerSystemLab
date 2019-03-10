#include "write.h"
#include "ui_write.h"

Write::Write(QString str, QWidget *parent) :
    QWidget(parent),  file(str),
    ui(new Ui::Write)
{
    ui->setupUi(this);
    b_empty = semget(B_EMPTY, 1, 0666);
    b_valid = semget(B_VALID, 1, 0666);
    // get shared memory
    int b_id = shmget(SHM_B, sizeof(int), 0666);
    b_mem = (Block *) shmat(b_id, nullptr, 0666);

    connect(this, &Write::log, ui->text_brw, &QTextBrowser::insertPlainText);
    t = new std::thread(&Write::write, this);
}

Write::~Write()
{
    if (t->joinable()) {
        t->join();
    }
    delete t;
    shmdt(b_mem);
    delete ui;
}

void Write::write()
{
    int fd = open(file.toStdString().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    int idx = 0;
    emit(log("Write start\n"));
    while (true) {
        P(b_valid);
        ::write(fd, b_mem->data, b_mem->len);
        V(b_empty);
        emit(log(QString("write ") + QString::number(idx) + " block from b_mem\n"));
        idx++;
        if (b_mem->end) {
            break;
        }
        sleep(rand() % 4);
    }
    emit(log("Write finished\n"));
}
