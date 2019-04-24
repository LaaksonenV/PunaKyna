#include "csvparser.h"
#ifdef BUILD_DEF
#include "build_def.h"
#else
#ifndef BUILD_VERSION
#define BUILD_VERSION 1
#endif
#endif

#include <QMessageBox>
#include <QRegExp>
#include <QtDebug>

CSVParser::CSVParser()
    : _file(nullptr)
    , _status(NotSet)
    , _emptyLine(QStringList())
    , _table(QList<QStringList>())
    , _changed(false)
    , _loadedVersion(QString())
    , _firstPrint(true)
{
}

CSVParser::CSVParser(const QString &file, bool lock, bool overwrite, bool backup)
    : CSVParser()
{
    if (!file.isEmpty())
        setFile(file, lock, overwrite, backup);
}

CSVParser::~CSVParser()
{
    lockFile(false);
    if (_file)
    {
        if (!_firstPrint)
            _file->close();
        delete _file;
    }

}

int CSVParser::setFile(const QString &file, bool lock, bool overwrite, bool backup)
{    
    _backup = backup;
    if (_file)
        return AlreadySet;
    _file = new QFile(file);

    if (fileLocked())
    {
        if (QMessageBox::critical(nullptr, "Tiedosto lukittu", "Tiedosto " + file + " on avattu toisessa ohjelmassa.\n"
                                                        "Tässä versiossa tiedostojen käyttö useammassa ohjelmassa\n"
                                                        "ei ole sallittua. Mikäli olet varma että tämä on virhe\n"
                                  " (mm. jos ohjelma kaatui),\n"
                                                        "voit poistaa lukituksen. Muuten tässä versiossa ohjelma täytyy\n"
                                  "käynnistää uudelleen vapautuneiden tiedostojen lataamiseksi."
                              , "Ok", "Poista lukitus") == 0)
        {
            delete _file;
            _file = nullptr;
            return NotSet;
        }
    }

    if (!overwrite && _file->exists())
    {
        if (!_file->open(QIODevice::ReadOnly|QIODevice::Text))
        {
            delete _file;
            _file = nullptr;
            return FileError;
        }
        QTextStream stream(_file);
        parseCSV(stream);
    }
    else
    {
        if (!_file->open(QIODevice::WriteOnly|QIODevice::Truncate))
        {
            delete _file;
            _file = nullptr;
            return FileError;
        }
        _loadedVersion = BUILD_VERSION;
    }
    _file->close();
    lockFile(lock);
    _status = Ready;
    return Ready;
}

void CSVParser::lockFile(bool lock)
{
#ifdef LOCK_FILES
    QFile lck(_file->fileName() + ".LOCK");
    if (!lock)
    {
        lck.remove();
        return;
    }
    lck.open(QFile::WriteOnly);
    lck.close();
#else
    Q_UNUSED(lock)
#endif //LOCK_FILES
}

bool CSVParser::fileLocked()
{
#ifdef LOCK_FILES
    if (QFile::exists(_file->fileName() + ".LOCK"))
        return true;
    return false;
#else
    return false;
#endif //LOCK_FILES
}

int CSVParser::fileSet() const
{
    return _status;
}

int CSVParser::rowCount() const
{
    return _table.count();
}

int CSVParser::columnCount() const
{
    if (_table.isEmpty())
        return 0;
    return _table.at(0).count();
}

const QString &CSVParser::getData(int row, int column) const
{
    if (column < 0 || row < 0)
        return errorMSG;
    while (_emptyLine.size() <= column)
    {
//        addColumn(QStringList());
        qWarning() << Q_FUNC_INFO << "column overindex" << column << "/"
                   << _emptyLine.size();
        return errorMSG;
    }
    while (_table.size() <= row)
    {
//        addRow(_emptyLine);
        qWarning() << Q_FUNC_INFO << "row overindex" << row << "/"
                   << _table.size();
        return errorMSG;
    }

    QStringList ret = _table.at(row);
    if (ret.size() != _emptyLine.size())
        qWarning() << Q_FUNC_INFO << "csv size discrepancies";
    return ret.at(column);
}

QStringList CSVParser::getColumn(int column, int fromRow) const
{
    if (_emptyLine.size() <= column)
    {
//        addColumn(QStringList());
        qWarning() << Q_FUNC_INFO << "column overindex" << column << "/"
                   << _emptyLine.size();
        return errorMSGL;
    }
    if (_table.size() <= fromRow)
    {
        qDebug() << Q_FUNC_INFO << "row overindex" << fromRow << "/"
                   << _table.size();
        return errorMSGL;
    }
    QStringList ret;
    QStringList row;
    bool check = false;
    for (int i = fromRow; i < _table.size(); ++i)
    {
        row = _table.at(i);
        if (row.size() <= column)
            ret << "";
        else
        {
            ret << row.at(column);
            check = true;
        }
    }
    if (!check)
        qWarning() << Q_FUNC_INFO << "csv size discrepancies";

    return ret;
}

const QStringList &CSVParser::getRow(int row) const
{
    while (_table.size() <= row)
    {
//        addRow(_emptyLine);
        qWarning() << Q_FUNC_INFO << "row overindex" << row << "/"
                   << _table.size();
        return errorMSGL;
    }
    return _table.at(row);
}

int CSVParser::findRow(const QString &data, int column, int fromRow) const
{
    if (column >= _emptyLine.size())
    {
        qWarning() << Q_FUNC_INFO << "column overindex" << column << "/"
                << _emptyLine.size();
        return -1;
    }

    QRegExp rx(data);
    for (int i = fromRow; i < _table.size(); ++i)
        if (rx.indexIn(_table.at(i).at(column)) >= 0)
            return i;
    return -1;
}

int CSVParser::findExactRow(const QString &data, int column, int fromRow) const
{
    if (column >= _emptyLine.size())
    {
        qWarning() << Q_FUNC_INFO << "column overindex" << column << "/"
                << _emptyLine.size();
        return -1;
    }

    for (int i = fromRow; i < _table.size(); ++i)
        if (_table.at(i).at(column) == data)
            return i;
    return -1;
}

int CSVParser::findColumn(const QString& data, int row, int fromCol) const
{
    if (row >= _table.size())
    {
        qWarning() << Q_FUNC_INFO << "row overindex" << row << "/"
                   << _table.size();
        return -1;
    }

    QRegExp rx(data);
    for (int i = fromCol; i < _emptyLine.size(); ++i)
        if (rx.indexIn(_table.at(row).at(i)) >= 0)
            return i;
    return -1;
}

int CSVParser::findExactColumn(const QString& data, int row, int fromCol) const
{
    if (row >= _table.size())
    {
        qWarning() << Q_FUNC_INFO << "row overindex" << row << "/"
                   << _table.size();
        return -1;
    }

    for (int i = fromCol; i < _emptyLine.size(); ++i)
        if (_table.at(row).at(i) == data)
            return i;
    return -1;
}

const QString &CSVParser::getVersion() const
{
    return _loadedVersion;
}

void CSVParser::changeData(const QString &data, int row, int column)
{
    while (_emptyLine.size() <= column)
        addColumn(QStringList());
    while (_table.size() <= row)
        addRow(_emptyLine);

    QStringList &tablerow = _table[row];
    tablerow[column] = data;
    _changed = true;
}

int CSVParser::addRow(const QStringList &data)
{
    int diff = data.count()-_emptyLine.count();
    if (diff > 0)
    {
        for (int j = diff; j > 0; --j)
        {
            _emptyLine.append("");
        }
        for (int i = 0; i < _table.count(); ++i)
        {
            for (int j = diff; j > 0; --j)
            {
                _table[i].append("");
            }
        }
    }
    _table.append(data);
//    for (int i = data.count(); i < _emptyLine.count(); ++i)
        for (int j = -diff; j > 0; --j)
            _table.last().append("");
    _changed = true;
    return _table.count()-1;
}

bool CSVParser::rearrangeData(int row, int to)
{
    if (row >= _table.size() || to >= _table.size())
        return false;
    _table.insert(to, _table.takeAt(row));
    _changed = true;
    return true;
}

bool CSVParser::removeRow(int row)
{
    if (row >= _table.size())
        return false;
    _table.removeAt(row);
    _changed = true;
    return true;
}

int CSVParser::addColumn(const QStringList &data)
{
//    int oldsize = _table.count();
    for (int i = _table.count(); i < data.count(); ++i)
        _table.append(_emptyLine);
    int i = 0;
    while (i < data.count())
    {
        _table[i].append(data.at(i));
        ++i;
    }
    while (i < _table.count())
    {
        _table[i].append("");
        ++i;
    }

    _emptyLine.append("");

    _changed = true;
    return _emptyLine.count()-1;
}

bool CSVParser::printCSV(bool markVersion)
{
    if (!_changed)
        return true;
    if (!_file)
        _file = new QFile("TEMP.csv");

    if (_firstPrint)
    {
        if (_backup)
        {
        if (QFile::exists(_file->fileName() + ".BCUP"))
            QFile::remove(_file->fileName() + ".BCUP");
        _file->copy(_file->fileName() + ".BCUP");
        }
        if (!_file->open(
                    QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        {
            _file->setFileName(_file->fileName() + "FILEOPENERROR.csv");
            if (!_file->open(
                    QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
                return false;
        }
        _firstPrint = false;
    }
    else
        _file->resize(0);

    QTextStream out(_file);

    out.setCodec("UTF-8");

#ifdef MARK_CSV
    if (markVersion)
        out << BUILD_VERSION_PRINT_PREFIX
            << BUILD_VERSION << "\n";
#else
    Q_UNUSED(markVersion)
#endif //MARK_CSV

    foreach (QStringList l, _table)
        out << outputText(l) << "\n";
    _changed = false;

//    out.setAutoDetectUnicode(true);

    out.flush();
    return true;
}

void CSVParser::printBackupCSV(const QString &extension, bool markVersion)
{
    if (!_file )
        return;

    QFile bckup(_file->fileName() + extension);

    if (bckup.exists())
        bckup.resize(0);

    if (!bckup.open(
                QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
        return;

    QTextStream out(&bckup);

    out.setCodec("UTF-8");

#ifdef MARK_CSV
    if (markVersion)
        out << BUILD_VERSION_PRINT_PREFIX
            << BUILD_VERSION << "\n";
#else
    Q_UNUSED(markVersion)
#endif //MARK_CSV

    foreach (QStringList l, _table)
        out << outputText(l) << "\n";

//    out.setAutoDetectUnicode(true);

    bckup.close();
}

bool CSVParser::parseCSV(QTextStream &text)
{
    text.setCodec("UTF-8");

    QString cell;
    QStringList parsedRow;
    QStringList tempStringList;
    QRegExp axp("^\"[^\"]");
    QRegExp lxp("[^\"]\"$");
    int width = 0;
    bool correct = true;
#ifdef MARK_CSV
    bool first_line = true;
    if (text.atEnd())
        _loadedVersion = BUILD_VERSION;
#endif //MARK_CSV
    while (!text.atEnd())
    {
        parsedRow.clear();
        cell = text.readLine();
        if (cell.isEmpty())
            continue;
        if (cell.count("\"") % 2 != 0)
            cell.append("\"");
        tempStringList = cell.split(',');
        while (!tempStringList.isEmpty())
        {
            cell = tempStringList.takeFirst();
            if (cell.count("\"") % 2 != 0)
            {
                cell.remove(0,1);
                while (cell.count("\"") % 2 == 0)
                {
                    cell += ',';
                    cell += tempStringList.takeFirst();
                }
                cell.remove(cell.count()-1,1);
            }
            if (axp.indexIn(cell) >= 0 && lxp.indexIn(cell) >= 0)
            {
                cell.remove(0,1);
                cell.remove(cell.count()-1,1);
            }
            cell.replace("\"\"", "\"");
            parsedRow << cell;
        }


#ifdef MARK_CSV
        if (first_line)
        {
            if (parsedRow.count() &&
                parsedRow.at(0).startsWith(BUILD_VERSION_PRINT_PREFIX))
            {
                _loadedVersion = parsedRow.at(0);
                _loadedVersion.remove(BUILD_VERSION_PRINT_PREFIX);
                first_line = false;

                continue;
            }
            _loadedVersion = "0.0";
            first_line = false;
        }
#endif //MARK_CSV
        if (width < parsedRow.count())
            width = parsedRow.count();
        _table.append(parsedRow);
    }
    for (int i = 0; i < _table.count(); ++i)
        while (_table.at(i).count() < width)
            _table[i].append("");
    while (_emptyLine.count() < width)
    {
        _emptyLine.append("");
    }
    return correct;
}

QString CSVParser::outputText(QStringList &list) const
{
    QString ret;
    bool first = true;
    foreach (QString s, list)
    {
        if (!first)
            ret += ",";
        /*By CSV "rules", each '"' entered by user should be double '""',
         * each comma inside quotations ("") is skipped by structure
         */
        if(s.contains("\""))
            s.replace("\"", "\"\"");
        if (s.contains(','))
            ((ret += '\"') += s) += '\"';
        else
            ret += s;
        first = false;
    }
    return ret;
}

QString &CSVParser::editData(int row, int column)
{
    return _table[row][column];
}
