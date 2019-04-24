#include "csvversionresolver.h"
#include "build_def.h"

#include "csvparser.h"
#include "settings.h"

#include <QRegExp>

CSVVersionResolver::CSVVersionResolver(Settings *set)
    : _settings(set)
    , _file(nullptr)
    , _type(CSVType_Null)
{
}

bool CSVVersionResolver::resolve(CSVParser *file, CSVType type)
{
    _file = file;
    _type = type;
    if (checkVersion() != Current)
    {
        return resolveFile(checkVersion());
    }
    return false;
}

int CSVVersionResolver::checkVersion()
{
    QString vers = _file->getVersion();
    QRegExp rx("v(\\d+)\\.(\\d+)\\.(\\d+)");
    if (vers == "v1.5 ALPHA")
        return v1_5;
    else if (vers == "v1.6 ALPHA")
        return v1_6;

    else if (rx.indexIn(vers) && rx.cap(1) == "1")
    {
        if (rx.cap(2).toInt() <= 7)
        {
            if(rx.cap(3).toInt() <= 4)
                return v1_74;
            else if (rx.cap(3).toInt() <= 7)
                return v1_77;
            else if (rx.cap(3).toInt() <= 8)
                return v1_78;
            else return  v1_7f;
        }
    }

    return Current;
}

bool CSVVersionResolver::resolveFile(int fromVersion)
{
    _settings->writeDebug(" Versio korjaus ");


    QString cell;
    QStringList splitCell;
    if (fromVersion <= v1_5 && _type == CSVType_Exam)
    {
        for (int i = 1; i < _file->rowCount(); ++i)
        for (int j = 1; j < _file->columnCount(); ++j)
        {
            cell = _file->getData(i,j);
            if (!cell.isEmpty())
            {
                splitCell = cell.split(v1_5_Archive::cellSeparator);
                splitCell[ExamFile::CommentCell_Status] =
                     GradingState::stateToChar(
                            splitCell[ExamFile::CommentCell_Status].toInt()+1);
                cell = splitCell.join(Printing::separator);
                _file->changeData(cell,i,j);
            }
        }
        return true;
    }
    else if (fromVersion <= v1_74 && _type == CSVType_Exam)
    {
        QRegExp rx("\\s(\\d+)p.");
        QString points;
        for (int i = 1; i < _file->rowCount(); ++i)
        for (int j = 1; j < _file->columnCount(); ++j)
        {
            cell = _file->getData(i,j);
            splitCell = cell.split(Printing::separator);
            for (int i = splitCell.count(); i < v1_74_Archive::CommentCell_By; ++i)
                splitCell << "";
            cell = splitCell.at(v1_74_Archive::CommentCell_Free);
            if (cell.isEmpty())
                points = "";
            else
            {
                rx.indexIn(cell);
                points = rx.cap(1);
                if (points.isEmpty())
                points = "0";
            }

            splitCell.insert(v1_74_Archive::CommentCell_By, points);
            cell = splitCell.join(Printing::separator);
            _file->changeData(cell,i,j);

        }
    }
    else if (fromVersion <= v1_78 && _type == CSVType_Question)
    {
        if (!_file->getData(0,v1_78_Archive::Meta_AutoControl).isEmpty())
        {
            _file->changeData(_file->getData(0,v1_78_Archive::Meta_NoControl),
                              0, QuestionFile::Meta_NoControl);
            _file->changeData(_file->getData(0,v1_78_Archive::Meta_AutoControl),
                              0, QuestionFile::Meta_AutoControl);
            _file->changeData("", 0, QuestionFile::Meta_Divider);

        }
    }
    return false;
}

