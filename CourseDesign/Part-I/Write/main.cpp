#include "write.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Write w(argv[2]);
    w.show();

    return a.exec();
}
