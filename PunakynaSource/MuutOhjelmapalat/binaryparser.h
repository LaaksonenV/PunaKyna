#ifndef BINARYPARSER_H
#define BINARYPARSER_H

class QString;
#include <QtGlobal>

class BinaryParser
{
public:
    typedef quint64 bins;

    static void addBinary(bins &to, int number);
    static bool checkBinary(bins &of, int number);

    static bins makeBinary(QString number, char separator = '#');
    static QString parseBinary(bins number, char separator = '#');

    BinaryParser();
    BinaryParser(bins binary);
    BinaryParser(QString binary, char separator = '#');

    void setBinary(bins binary);
    void setBinary(QString binary, char separator = '#');
    int getNext();
    bool takeNext(int &to);

private:
    bins m_number;

};

#endif // BINARYPARSER_H
