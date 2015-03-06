#include <QApplication>
#include "denoising.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Denoising w;
    w.show();
    
    return a.exec();
}
