#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    read_process = new QProcess(this);
    copy_process = new QProcess(this);
    print_process = new QProcess(this);
    read_process->setStandardOutputFile("read_out.txt");
    copy_process->setStandardOutputFile("copy_out.txt");
    print_process->setStandardOutputFile("print_out.txt");
    // accquire some shared memory if memory is not ready
    a_empty = semget(A_EMPTY, 1, IPC_CREAT | 0666);
    a_valid = semget(A_VALID, 1, IPC_CREAT | 0666);
    b_empty = semget(B_EMPTY, 1, IPC_CREAT | 0666);
    b_valid = semget(B_VALID, 1, IPC_CREAT | 0666);
    shm_a = shmget(SHM_A, sizeof(Block), IPC_CREAT | 0666);
    shm_b = shmget(SHM_B, sizeof(Block), IPC_CREAT | 0666);
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
    // set default valuee
    semun arg;
    arg.val = 0;
    semctl(a_valid, 0, SETVAL, arg);
    semctl(b_valid, 0, SETVAL, arg);
    arg.val = 1;
    semctl(a_empty, 0, SETVAL, arg);
    semctl(b_empty, 0, SETVAL, arg);

    QStringList params;
    params << ui->src_edit->text() << ui->dst_edit->text();
    qDebug() << ui->src_edit->text() << "\n";
    QProcess *ps[3] = {read_process, copy_process, print_process};
    const char *programs[3] = {"Read", "Copy", "Write"};
    for (int i = 0; i < 3; ++i) {
        // start process
        if (ps[i]->state() == QProcess::Running) {
            ps[i]->kill();
            ps[i]->waitForFinished();
        }
        ps[i]->start(programs[i], params);
    }
}

void Widget::on_src_btn_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select source file", "/home", "*");
    if (!file.isEmpty()) {
        ui->src_edit->setText(file);
    }
}

void Widget::on_dst_btn_clicked()
{
    QString file = QFileDialog::getSaveFileName(this, "Select save file", "/home", "*");
    if (!file.isEmpty()) {
        ui->dst_edit->setText(file);
    }
}
