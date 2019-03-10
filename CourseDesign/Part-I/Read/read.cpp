#include "read.h"
#include "ui_read.h"

Read::Read(QString file, QWidget *parent) :
    QWidget(parent),file(file),
    ui(new Ui::Read)
{
    ui->setupUi(this);
    // start
    a_empty = semget(A_EMPTY, 1, 0666);
    a_valid = semget(A_VALID, 1, 0666);
    // get shared memory
    a_id = shmget(SHM_A, sizeof(Block), 0666);
    mem = (Block*)shmat(a_id, nullptr, 0666);
    connect(this,  &Read::log, ui->text_brw, &QTextBrowser::insertPlainText);
    t = new std::thread(&Read::read_file, this);
}

Read::~Read()
{
    if (t->joinable()) {
        t->join();
    }
    delete t;
    shmdt(mem);
    delete t;
    delete ui;
}

void Read::read_file()
{
    int fd = open(file.toStdString().c_str(), O_RDONLY);
    std::cout << "file = " << file.toStdString() << std::endl;
    std::cout << "get fd = " << fd << std::endl;
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(BUF_SIZE);
    int idx = 0;
    emit(log("Read start\n"));
    while (true) {
        int len = read(fd, buf.get(), BUF_SIZE);
        std::cout << "len = " << len << std::endl;
        P(a_empty);
        memcpy(mem->data, buf.get(), len);
        mem->end = len < BUF_SIZE;
        mem->len = len;
        V(a_valid);
        emit(log(QString("read ") + QString::number(idx) + " block to a_mem\n"));
        idx++;
        if (mem->end) {
            break;
        }
        sleep(rand() % 4);
    }
    emit(log("Read finished\n"));
}


