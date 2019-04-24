#include "binaryparser.h"

#include <QStringList>
#include <climits>

void BinaryParser::addBinary(bins &to, int number)
{
    to ^= (bins(1) << number);
    return;
}

bool BinaryParser::checkBinary(bins &of, int number)
{
    if (of & (bins(1) << number))
        return true;
    return false;
}

BinaryParser::bins BinaryParser::makeBinary(QString number, char separator)
{
    QStringList numList = number.split(separator, QString::SkipEmptyParts);
    BinaryParser::bins ret = 0;
    int temp = 0;
    foreach (QString s, numList)
    {
        temp = s.toInt();
        if (temp < int(sizeof(bins) * CHAR_BIT))
            ret += (bins(1) << temp);
    }
    return ret;
}

QString BinaryParser::parseBinary(bins number, char separator)
{
    QStringList ret;
    unsigned long i = 0;
    while (i < sizeof(bins) * CHAR_BIT && number >= (bins(1) << i))
    {
        if (checkBinary(number, i))
        {
            ret << QString::number(i);
        }
        ++i;
    }
    return ret.join(separator);
}

BinaryParser::BinaryParser()
    : m_number(0)
{}

BinaryParser::BinaryParser(bins binary)
    : m_number(binary)
{}

BinaryParser::BinaryParser(QString binary, char separator)
    : BinaryParser(BinaryParser::makeBinary(binary, separator))
{}

void BinaryParser::setBinary(bins binary)
{
    m_number = binary;
}

void BinaryParser::setBinary(QString binary, char separator)
{
    m_number = makeBinary(binary, separator);
}

int BinaryParser::getNext()
{
    if (!m_number)
        return -1;
    BinaryParser::bins i = 0;
    while (!(m_number & (bins(1) << i)) && i < sizeof(bins)*CHAR_BIT)
        ++i;
    if (i >= sizeof(bins)*CHAR_BIT)
        return -1;
    addBinary(m_number, i);
    return i;
}

bool BinaryParser::takeNext(int &to)
{
    int ret = getNext();
    if (ret < 0)
        return false;
    to = ret;
    return true;
}
