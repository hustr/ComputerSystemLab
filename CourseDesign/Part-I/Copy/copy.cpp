#include "copy.h"
#include "ui_copy.h"

Copy::Copy(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Copy)
{
    ui->setupUi(this);
    a_empty = semget(A_EMPTY, 1, 0666);
    a_valid = semget(A_VALID, 1, 0666);
    b_empty = semget(B_EMPTY, 1, 0666);
    b_valid = semget(B_VALID, 1, 0666);
    // get shared memory
    int a_id = shmget(SHM_A, sizeof(Block), 0666);
    int b_id = shmget(SHM_B, sizeof(Block), 0666);

    a_mem = (Block *) shmat(a_id, nullptr, 0666);
    b_mem = (Block *) shmat(b_id, nullptr, 0666);
    connect(this, &Copy::log, ui->text_brw, &QTextBrowser::insertPlainText);
    t = new std::thread(&Copy::copy, this);
}

Copy::~Copy()
{
    if (t->joinable()) {
        t->join();
    }
    delete t;
    shmdt(a_mem);
    shmdt(b_mem);
    delete ui;
}

void Copy::copy()
{
    emit(log("Copy start\n"));
    int idx = 0;
    std::unique_ptr<Block> buf = std::make_unique<Block>();
    while (true) {
        P(a_valid);
        memcpy(buf.get(), a_mem, sizeof(Block));
        V(a_empty);
        emit(log(QString("copy ") + QString::number(idx) + " block to buf\n"));
        P(b_empty);
        memcpy(b_mem, buf.get(), sizeof(Block));
        V(b_valid);
        emit(log(QString("copy ") + QString::number(idx) + " block to b_mem\n"));
        idx++;
        if (buf->end) {
            break;
        }
        sleep(rand() % 4);
    }
    emit(log("Copy finished\n"));
}


