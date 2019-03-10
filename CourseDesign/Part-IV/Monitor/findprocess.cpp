#include "findprocess.h"
#include "ui_findprocess.h"

FindProcess::FindProcess(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindProcess)
{
    ui->setupUi(this);
    ui->kill_btn->setEnabled(false);
    ui->result_widget->hide();
    connect(ui->search_btn, &QPushButton::clicked, this, &FindProcess::search_process);
    connect(ui->pid_edit, &QLineEdit::textChanged, this, [&](){
        ui->search_btn->setEnabled(false);
        // check if pid text valid
        QString pid_text = ui->pid_edit->text();
        pid_text = pid_text.trimmed();
        bool ok;
        int val = pid_text.toInt(&ok);
        if (!ok || val <= 0) {
            // not valid
            ui->search_btn->setEnabled(false);
        }
        else {
            ui->search_btn->setEnabled(true);
        }
    });
    connect(ui->kill_btn, &QPushButton::clicked, this, &FindProcess::kill_process);
}

FindProcess::~FindProcess()
{
    delete ui;
}

std::unique_ptr<char[]> convert_bytes(unsigned long bytes) {
    // calculate and transfer bytes to kb and mb
    std::unique_ptr<char[]> res = std::make_unique<char[]>(40);

    if (bytes >= 1024 * 1024) {
        sprintf(res.get(), "%.2f MB", bytes / 1024.0 / 1024.0);
    } else {
        sprintf(res.get(), "%.2f KB", bytes / 1024.0);
    }
//    qDebug() << "bytes: " << bytes << " res: " << res.get();
    return res;
}

void FindProcess::search_process()
{
    ui->kill_btn->setEnabled(false);
    ui->result_widget->hide();
    int id = ui->pid_edit->text().trimmed().toInt();
    bool ok;
    process_t p = get_process_info(id, &ok);
    // print message to the display
    QString s;

    if (!ok) {
        char msg[100]{};
        sprintf(msg, "Sorry, Monitor cannot find the process %d, please enter correct pid", id);
        QMessageBox::information(this, "", msg);
        return;
    }
    ui->kill_btn->setEnabled(true);
    // ok set the message
    ui->name_label->setText(p.comm);
    ui->user_label->setText(get_user(id).get());
    ui->status_label->setText(status_char_to_str(p.state));
    // memory calculate
    ui->mem_label->setText(convert_bytes(static_cast<unsigned long>(p.resident - p.share) * PAGE_SIZE).get());
    ui->vmem_label->setText(convert_bytes(p.vsize).get());
    ui->rss_label->setText(convert_bytes(static_cast<unsigned long>(p.rss) * PAGE_SIZE).get());
    ui->smem_label->setText(convert_bytes(static_cast<unsigned long>(p.share) * PAGE_SIZE).get());
//    qDebug() << p.starttime;
    char time[100]{};
    tm start_tm = *get_start_time();
    start_tm.tm_sec += p.starttime / 100;
    mktime(&start_tm);

//    clock_t ck = static_cast<clock_t>(p.starttime);
    strftime(time, sizeof(time) - 1, TM_FORMAT, &start_tm);
    ui->btime_label->setText(time);
    ui->nice_label->setText(QString::number(p.nice));
    ui->pri_label->setText(QString::number(p.priority));
    ui->id_label->setText(QString::number(p.pid));

    ui->result_widget->show();
}

void FindProcess::kill_process()
{
    if ( QMessageBox::Ok != QMessageBox::warning(this, "Warning!", "Are you sure to stop process " + ui->pid_edit->text().trimmed(),
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel)) {
        return;
    }
    // kill process
    QProcess p;
    QList<QString> args;
    args << "-SIGKILL" << ui->pid_edit->text().trimmed();
    p.startDetached("kill", args);
}

void FindProcess::show()
{
    ui->pid_edit->clear();
    ui->kill_btn->setEnabled(false);
    ui->result_widget->hide();
    QWidget::show();
}
