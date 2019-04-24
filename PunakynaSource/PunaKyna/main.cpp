#include "build_def.h"
#include "settings.h"

#include "mainwindow.h"
#include <QApplication>
#include <exception>


int main(int argc, char *argv[])
{
    Settings *set;
    int ret = 1;
    try
    {
        QApplication a(argc, argv);
        set = new Settings(argv[0]);
        MainWindow w(set);
        w.show();
        ret = a.exec();

    }
    catch(...)
    {
        if (set)
            set->printOnError();
    }
    if (set)
        delete set;
    return ret;
}
