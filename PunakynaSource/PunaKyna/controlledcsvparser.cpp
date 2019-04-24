#include "controlledcsvparser.h"

ControlledCSVParser::ControlledCSVParser()
    : CSVParser()
{
    _gradingsAt = 0;
}

bool ControlledCSVParser::setFile(const QString &file)
{
    bool appending = false;
    int old = rowCount();
    if (fileSet())
        appending = true;
    if (!CSVParser::setFile(file))
        return false;

    _gradingsAt = findData(_commentEndMSG, 0) +1;

    if (appending)
        bookNewcomers(old);

    return true;
}

int ControlledCSVParser::commentCount()
{
    return _gradingsAt-1;
}

const QStringList &ControlledCSVParser::getComment(int row)
{
    if (row >= _gradingsAt-1)
        return errorMSGL;
    return getRow(row);
}

bool ControlledCSVParser::changeComment(const QString &data, int row,
                                        Grading::CommentTable column)
{
    if (row >= _gradingsAt-1)
        return false;
    CSVParser::changeData(data, row, (int)column);
    return true;
}

void ControlledCSVParser::addComment(const QStringList &data)
{
    CSVParser::addRow(data);
    if (!_gradingsAt)
    {
        CSVParser::addRow(QStringList(_commentEndMSG));
        _gradingsAt = 2;
    }
    else
        ++_gradingsAt;
}

int ControlledCSVParser::rowCount()
{
    return CSVParser::rowCount()-_gradingsAt;
}

int ControlledCSVParser::addRow(const QStringList &data)
{
    return CSVParser::addRow(data)-_gradingsAt;
}

QString ControlledCSVParser::getData(int row, int column)
{
    return CSVParser::getData(row-_gradingsAt, column);
}

QStringList ControlledCSVParser::getColumn(int column, int fromRow)
{
    QStringList ret = CSVParser::getColumn(column, fromRow);
    for (int i = fromRow; i < CSVParser::rowCount() && i < _gradingsAt; ++i)
        ret.removeFirst();
    return ret;
}

const QStringList &ControlledCSVParser::getRow(int row)
{
    return CSVParser::getRow(row-_gradingsAt);
}

void ControlledCSVParser::changeData(const QString &data, int row, int column)
{
    CSVParser::changeData(data, row-_gradingsAt, column);
}

bool ControlledCSVParser::assignComment(unsigned short id, int to)
{
    QString com = QString::number(id);
    com.prepend('#'); com.append('#');
    QString &data = editData(to, (int)Grading::GradeTable::Comment);
    if (data.contains(com))
    {
        data.remove(com);
        return false;
    }
    else
        data.append(com);
    return true;
}

QVector<unsigned short> ControlledCSVParser::getComments(int of)
{
    QString data = getData(of, (int)Grading::GradeTable::Comment);
    if (data == "")
        return QVector<unsigned short>();
    QVector<unsigned short> ret;
    data.remove(0, 1);
    data.remove(data.size()-1, 1);
    QStringList temp = data.split("##");
    for (int i = 0; i < temp.size(); ++i)
        ret.append(temp.at(i).toShort());
    return ret;
}

void ControlledCSVParser::bookNewcomers(int newcomerCommentsStart)
{
    int until = findData(_commentEndMSG,0 , newcomerCommentsStart);
    if (until < 0)
        return;
    int oldId = 0;
    while (newcomerCommentsStart < until)
    {
        rearrangeData(newcomerCommentsStart, _gradingsAt-1);
        changeCommentedId(oldId, _gradingsAt-1, until+1);
        ++_gradingsAt;
        ++newcomerCommentsStart;
        ++oldId;
    }
}

void ControlledCSVParser::changeCommentedId(int oldId, int newId, int fromRow)
{
    QStringList toChangeList = CSVParser::getColumn((int)Grading::GradeTable::Comment,
                                                fromRow);
    QString toChange;
    int i = 0;
    while (fromRow+i < rowCount())
    {
        toChange = toChangeList.at(i);
        if (toChange.contains(QString::number(oldId)))
        {
            toChange.replace(QString::number(oldId),
                             QString::number(newId));
            CSVParser::changeData(toChange, fromRow+i,
                                 (int)Grading::GradeTable::Comment);
            ++i;
        }
    }
}
