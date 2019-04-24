#include "conflictresolver.h"

#include <QMap>
#include <QTreeWidget>

#include "settings.h"

void ConflictResolver::mergeExams(CSVParser &original, CSVParser &added)
{
    QStringList firstRow = added.getRow(0);
    QStringList originalRow = original.getRow(0);
    int column = 0;
    int row = 0;
    for (int aCol = 1; aCol < firstRow.count(); ++aCol)
    {
        column = 1;
        while (column < originalRow.count() && originalRow.at(column) != firstRow.at(aCol))
            ++column;
        if (column >= originalRow.count())
            original.addColumn(added.getColumn(aCol));
        else
        {
            for (int aRow = 1; aRow < added.rowCount(); ++aRow)
            {
                if (added.getData(aRow,aCol) == "")
                    continue;
                row = original.findRow(added.getData(aRow,0),0,1);
                if (row < 0)
                    original.addRow(added.getRow(aRow));
                else
                {
                    /*if (original.getData(row, column) != "" &&
                        original.getData(row, column) != added.getData(aRow, aCol))
                        {
                        }
                    else*/
                    original.changeData(added.getData(aRow, aCol),row, column);
                }
            }
        }
    }
}

ConflictResolver::ConflictResolver(CSVParser &original, CSVParser &added, QWidget *parent)
    : QDialog(parent)
    , _mapping(QMap<int,int>())
{
    // V.a1, ei liikaa tarkistuksia lisätään vain kaikki yhteen
    int row = 0;
    for (int i = 1; i < added.rowCount() && i+original.rowCount()-2 < 32; ++i)
    {
        if (different(original.getRow(i), added.getRow(i)))
        {
            original.addRow(added.getRow(i));
            row = original.rowCount()-1;
            original.changeData(QString::number(row-1), row,
                                QuestionFile::CommentRow_Id);
            _mapping.insert(i,row);
        }
        else
            _mapping.insert(i,i);
    }
}

int ConflictResolver::getMapped(int of)
{
    return _mapping.value(of);
}

bool ConflictResolver::different(QStringList row1, QStringList row2)
{
    for (int i = 1; i < QuestionFile::CommentRow_By &&
                    i < row1.count() && i < row2.count(); ++i)
        if (row1.at(i) != row2.at(i))
            return true;
    return false;
}
