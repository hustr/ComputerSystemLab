#include "copy.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Copy w;
    w.show();

    return a.exec();
}
