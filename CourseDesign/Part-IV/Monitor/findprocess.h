#ifndef FINDPROCESS_H
#define FINDPROCESS_H

#include <QWidget>
#include <QMessageBox>
#include <QProcess>
#include "utils.h"

namespace Ui {
class FindProcess;
}

class FindProcess : public QWidget
{
    Q_OBJECT

public:
    explicit FindProcess(QWidget *parent = nullptr);
    ~FindProcess();
private slots:
    void search_process();
    void kill_process();
public slots:
    void show();



private:
    Ui::FindProcess *ui;
};

#endif // FINDPROCESS_H
