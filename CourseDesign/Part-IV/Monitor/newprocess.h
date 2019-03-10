#ifndef NEWPROCESS_H
#define NEWPROCESS_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class NewProcess;
}

class NewProcess : public QDialog
{
    Q_OBJECT

public:
    explicit NewProcess(QWidget *parent = nullptr);
    ~NewProcess();
private slots:
    void on_buttonBox_accepted();
public slots:
    void show();
private:
    void new_process();
    Ui::NewProcess *ui;
};

#endif // NEWPROCESS_H
