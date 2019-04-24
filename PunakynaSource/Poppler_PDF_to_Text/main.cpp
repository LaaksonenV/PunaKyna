#include <iostream>
//#include <QCoreApplication>

#include "poppler-qt5.h"

#include <QString>

int main(int argc, char *argv[])
{
 //   QCoreApplication a(argc, argv);

    if (argc > 2)
    {
        std::cout << "Program can only parse one file per run";
        return 0;
    }
    QString file;
    if (argc == 2)
        file = argv[1];
    else
    {
        std::string strfile;
        std::getline(std::cin, strfile);
        file.fromStdString(strfile);
    }
    Poppler::Document *doc = Poppler::Document::load(file);
    if (!doc)
    {
        std::cerr << "File given could not be read";
        return 1;
    }
    const int pagesNbr = doc->numPages();

    Poppler::Page *pg;
    QString pageText;

    for (int i = 0; i < pagesNbr; ++i)
    {
        pg = doc->page(i);
        if (!pg)
            std::cerr << "NULL page created" << std::endl;
        else
        {
            pageText = pg->text(QRect());
            std::cout << pageText.toStdString() << std::endl;
        }
    }
//    std::cout << "END_OF_DOCUMENT" << std::endl;
    if (pg)
        delete pg;

//    return a.exec();
    return 0;
}

