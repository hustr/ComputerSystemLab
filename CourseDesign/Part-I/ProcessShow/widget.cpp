#include "widget.h"
#include "ui_widget.h"
#include "../config.hpp"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    read_process = new QProcess(this);
    copy_process = new QProcess(this);
    print_process = new QProcess(this);
    connect(read_process, &QProcess::readyReadStandardOutput, this, &Widget::read_output_ready);
    connect(copy_process, &QProcess::readyReadStandardOutput, this, &Widget::copy_output_ready);
    connect(print_process, &QProcess::readyReadStandardOutput, this, &Widget::print_output_ready);
}

Widget::~Widget()
{
    read_process->kill();
    copy_process->kill();
    print_process->kill();
    // wait for process to end
    read_process->waitForFinished();
    copy_process->waitForFinished();
    print_process->waitForFinished();
    delete read_process;
    delete copy_process;
    delete print_process;
    delete ui;
    // process memory and semaphore
    semctl(a_empty, 0, IPC_RMID);
    semctl(a_valid, 0, IPC_RMID);
    semctl(b_empty, 0, IPC_RMID);
    semctl(b_valid, 0, IPC_RMID);
    // be careful about the usage diffenence between shmctl and semctl 
    shmctl(shm_a, IPC_RMID, nullptr);
    shmctl(shm_b, IPC_RMID, nullptr);
}



void Widget::on_start_btn_clicked()
{
    // accquire some shared memory if memory is not ready
    a_empty = semget(A_EMPTY, 1, IPC_CREAT | 0666);
    a_valid = semget(A_VALID, 1, IPC_CREAT | 0666);
    b_empty = semget(B_EMPTY, 1, IPC_CREAT | 0666);
    b_valid = semget(B_VALID, 1, IPC_CREAT | 0666);
    // set default value
    semun arg;
    arg.val = 0;
    semctl(a_valid, 0, SETVAL, arg);
    semctl(b_valid, 0, SETVAL, arg);
    arg.val = 1;
    semctl(a_empty, 0, SETVAL, arg);
    semctl(b_empty, 0, SETVAL, arg);


    shm_a = shmget(SHM_A, sizeof(int), IPC_CREAT | 0666);
    shm_b = shmget(SHM_B, sizeof(int), IPC_CREAT | 0666);

    // clear output
    QString file = ui->filename_edit->text();
    QStringList params;
    params << file;
    ui->read_brw->clear();
    ui->copy_brw->clear();
    ui->print_brw->clear();
    QProcess *ps[3] = {read_process, copy_process, print_process};
    const char *programs[3] = {"../read/read", "../copy/copy", "../print/print"};
    for (int i = 0; i < 3; ++i) {
        // start process
        if (ps[i]->state() == QProcess::Running) {
            ps[i]->kill();
            ps[i]->waitForFinished();
        }
        ps[i]->start(programs[i]);
    }
}

// process output of read process
void Widget::read_output_ready()
{
    ui->read_brw->insertPlainText(read_process->readAllStandardOutput());
}

// process output of copy process
void Widget::copy_output_ready()
{
    ui->copy_brw->insertPlainText(copy_process->readAllStandardOutput());
}

// process output of print process
void Widget::print_output_ready()
{
    ui->print_brw->insertPlainText(print_process->readAllStandardOutput());
}
