#include "read.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    for (int i = 0;i < argc; ++i) {
        std::cout << argv[i] << std::endl;
    }
    Read w(argv[1]);
    w.show();

    return a.exec();
}
