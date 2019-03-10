#include "newprocess.h"
#include "ui_newprocess.h"

NewProcess::NewProcess(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProcess)
{
    ui->setupUi(this);
}

NewProcess::~NewProcess()
{
    delete ui;
}

void NewProcess::new_process()
{
    QString program = ui->program_edit->text();
    QProcess p;
    p.startDetached(program);
}

void NewProcess::on_buttonBox_accepted()
{
    new_process();
}

void NewProcess::show()
{
    ui->program_edit->clear();
    QWidget::show();
}
