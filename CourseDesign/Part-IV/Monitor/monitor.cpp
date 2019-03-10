#include "monitor.h"
#include "ui_monitor.h"

Monitor::Monitor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Monitor)
{
    ui->setupUi(this);
//    ui->statusBar->showMessage("Java Hello");
    // set status bar
    status_timer.reset(new QTimer());
    status_timer->setInterval(1000);
    connect(status_timer.get(), &QTimer::timeout, this, [&](){
        // update the status bar
        ui->status_label->setText(get_status_msg());
    });
    status_timer->start();

    // set cpu and mem update
    cpu_data.resize(POINTS_CNT, 0);
    mem_data.resize(POINTS_CNT, 0);
    swap_data.resize(POINTS_CNT, 0);

    cpu = new QSplineSeries();
    cpu->setName("CPU");
    mem = new QSplineSeries();
    mem->setName("Mem");
    swap = new QSplineSeries();
    swap->setName("Swap");

    ui->cpu_layout->addWidget(create_cpu_view(cpu));
    ui->cpu_layout->addWidget(create_mem_view(mem, swap));
    cpu_mem_timer.reset(new QTimer());
    cpu_mem_timer->setInterval(1000);
    connect(cpu_mem_timer.get(), &QTimer::timeout, this, &Monitor::update_chart);
    cpu_mem_timer->start();

    ui->host_label->setText(get_host());
    ui->os_label->setText(get_os_type());
    // btime
    char buf[100]{};
    strftime(buf, sizeof(buf) - 1, "%Y-%m-%d %H:%M:%S", get_start_time());
    ui->btime_label->setText(buf);

    // set information tab
//    up_timer.reset(new QTimer());
    // up_timer and cpu timer can use as the same
    connect(cpu_mem_timer.get(), &QTimer::timeout, this, [&](){
        // up time in seconds
        char buf[100]{};
        clock_t uptime = get_uptime();
        // format to day hour min seconds
        long day = uptime / DAY;
        uptime %= DAY;
        long hour = uptime / HOUR;
        uptime %= HOUR;
        long minute = uptime / MINUTE;
        uptime %= MINUTE;
        // left for seconds
        sprintf(buf, "%ldday(s) %ld:%ld:%ld", day, hour, minute, uptime);
        ui->uptime_label->setText(buf);
    });
    ui->model_label->setText(get_cpu_model());
    connect(cpu_mem_timer.get(), &QTimer::timeout, this, [&](){
         ui->freq_label->setText(QString::number(get_cpu_freq(), 'f', 2) + " MHz");
    });
    // update processes per second
    process_timer.reset(new QTimer());
    process_timer->setInterval(1000);
    connect(process_timer.get(), &QTimer::timeout, this, &Monitor::update_process);
    process_timer->start();
//    update_process();
    connect(ui->actionFind_Process, &QAction::triggered, this, &Monitor::find_process);
    connect(ui->actionShutdown, &QAction::triggered, this, &Monitor::shutdown);
    connect(ui->actionNew_Process, &QAction::triggered, this, &Monitor::new_process);
    new_p = new NewProcess();
    new_p->hide();
    find_p = new FindProcess();
    find_p->hide();
}

Monitor::~Monitor()
{
    new_p->close();
    find_p->close();
    delete new_p;
    delete find_p;
    delete ui;
}

void Monitor::find_process()
{
//    QMessageBox::information(nullptr, "Hello", "Hello world!");
    find_p->show();
}

void Monitor::shutdown()
{
    if (QMessageBox::warning(this, "Warning", "Are you sure to shutdown the system?",
                             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok) {
        QList<QString> args;
//        args << "-h" << "0";
        QProcess::execute("/usr/bin/shutdown -h 0");
    }
}

void Monitor::new_process()
{
    new_p->show();
}


QChartView *Monitor::create_cpu_view(QSplineSeries *cpu)
{
    cpu->setUseOpenGL(true);
    QChart *chart = new QChart();
    // add to a chart
    chart->addSeries(cpu);

    QCategoryAxis *axisX = new QCategoryAxis();
    QCategoryAxis *axisY = new QCategoryAxis();
    // set axis value
    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);
    axisX->setRange(0, 120);
    axisY->setRange(0, 100);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    for (int i = 10; i <= 120; i += 10) {
        axisX->append(QString::number(120 - i) + "s", i);
    }
    for (int i = 20; i <= 100; i += 20) {
        axisY->append(QString::number(i) + "%", i);
    }
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignRight);
    cpu->attachAxis(axisX);
    cpu->attachAxis(axisY);

    QChartView *view = new QChartView(chart);
    // antialiasing
    view->setRenderHint(QPainter::HighQualityAntialiasing);

    return view;
}

QChartView *Monitor::create_mem_view(QSplineSeries *mem, QSplineSeries *swap)
{
    mem->setUseOpenGL(true);
    swap->setUseOpenGL(true);

    QChart *chart = new QChart();
    // add to a chart
    chart->addSeries(mem);
    chart->addSeries(swap);

    QCategoryAxis *axisX = new QCategoryAxis();
    QCategoryAxis *axisY = new QCategoryAxis();
    // set axis value
    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);
    axisX->setRange(0, 120);
    axisY->setRange(0, 100);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisY->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    for (int i = 10; i <= 120; i += 10) {
        axisX->append(QString::number(120 - i) + "s", i);
    }
    for (int i = 20; i <= 100; i += 20) {
        axisY->append(QString::number(i) + "%", i);
    }
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignRight);
    mem->attachAxis(axisX);
    mem->attachAxis(axisY);
    swap->attachAxis(axisX);
    swap->attachAxis(axisY);

    QChartView *view = new QChartView(chart);
    // antialiasing
    view->setRenderHint(QPainter::HighQualityAntialiasing);

    return view;
}

void Monitor::update_chart()
{
    update_cpu_usage();
    update_mem_usage();
    // update cpu data and series
//    double usage = get_cpu_usage();
    cpu_data.pop_front();
    mem_data.pop_front();
    swap_data.pop_front();
    cpu_data.push_back(cpu_usage);
    mem_data.push_back(mem_usage);
    swap_data.push_back(swap_usage);
//    std::cout << "add data: " << cpu_data.back() << std::endl;
//    qDebug() << "add data: " << cpu_data.back() << "\n";
    cpu->clear();
    mem->clear();
    swap->clear();
    unsigned idx;
    for (int i = 0; i < POINTS_CNT; ++i) {
        idx = static_cast<unsigned>(i);
        *cpu << QPointF(i, cpu_data[idx]);
        *mem << QPointF(i, mem_data[idx]);
        *swap << QPointF(i, swap_data[idx]);
    }
}

void Monitor::update_process()
{
    // get all process
    std::vector<pid_t> pids = get_pids();
    std::map<pid_t, process_t> processes;
    for (pid_t &id : pids) {
        processes[id] = get_process_info(id);
    }
    // update the process table
    QTableView *tbl = ui->tableView;
    static std::once_flag once{};
    // process header
    std::call_once(once, [&](){
        model = new QStandardItemModel();
        static const std::vector<const char*> rows = {
            "pid",
            "name",
            "ppid",
            "user",
            "mem",
            "priority",
        };
        size_t size = rows.size();
        for (size_t i = 0; i < size; ++i) {
//            qDebug() << rows[i];
            model->setHorizontalHeaderItem(static_cast<int>(i), new QStandardItem(rows[i]));
        }
        tbl->setModel(model);
        QHeaderView *header = tbl->horizontalHeader();
        header->setSectionResizeMode(QHeaderView::Stretch);
        // set the pid column to constant
        header->setSectionResizeMode(0, QHeaderView::Fixed);
        header->setStretchLastSection(true);
        tbl->verticalHeader()->hide();
        tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    });
    model->removeRows(0, model->rowCount());
    // insert to row
    for (auto &p : processes) {
        // insert
        QList<QStandardItem*> item;
        item << new QStandardItem(QString::number(p.first)) // pid
             << new QStandardItem(p.second.comm) // command
             << new QStandardItem(QString::number(p.second.ppid)) // ppid
             << new QStandardItem(QString(get_user(p.second.pid).get())) // user
             << new QStandardItem(QString(calculate_memory(p.second).get())) // memory
             << new QStandardItem(QString::number(p.second.priority)); // priority
        model->insertRow(model->rowCount(), item);
    }
}
















