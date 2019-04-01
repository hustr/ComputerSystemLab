#ifndef MONITOR_H
#define MONITOR_H

#include <QMainWindow>
#include <QtCharts>
#include <QLayout>
#include <deque>
#include <QDebug>
#include <memory>
#include <QDialog>
#include "newprocess.h"
#include "findprocess.h"


#include "utils.h"
#define POINTS_CNT 121

namespace Ui {
class Monitor;
}

class Monitor : public QMainWindow
{
    Q_OBJECT

public:
    explicit Monitor(QWidget *parent = nullptr);
    ~Monitor();

private slots:
    void find_process();
    void shutdown();
    void new_process();

private:
    // create the data view use data series
    // main use for cpu, mem, swap
    static QChartView *create_cpu_view(QSplineSeries *cpu);
    static QChartView *create_mem_view(QSplineSeries *mem, QSplineSeries *swap);
    void update_chart();
    void update_process();
    // update once per 0.5s
//    int point_cnt = 240;
    std::deque<double> cpu_data, mem_data, swap_data;
    // managed by qt form
    QSplineSeries *cpu;
    QSplineSeries *mem;
    QSplineSeries *swap;
    // update timer
    std::unique_ptr<QTimer> cpu_mem_timer;
    std::unique_ptr<QTimer> process_timer;
    std::unique_ptr<QTimer> status_timer;

    QStandardItemModel *model;
//    int last_select;
//    std::unique_ptr<QTimer> up_timer;
    std::unique_ptr<NewProcess> new_p;
    std::unique_ptr<FindProcess> find_p;

    Ui::Monitor *ui;
};

#endif // MONITOR_H
