#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QVector>

class CSVParser
{
public:
    enum FileStatus
    {
        NotSet, Ready, ReadOnly, AlreadySet, FileError
    };

    CSVParser();
    CSVParser(const QString &file, bool lock = true, bool overwrite = false
            , bool backup = true);
    virtual ~CSVParser();

    /* setFile does nothing if the parser already is associated with
     * a file, or the file fails to open (only reason to call this
     * function from outside, as th class constructor calls it too).
     */
    virtual int setFile(const QString &file, bool lock = true,
                        bool overwrite = false, bool backup = true);
    virtual void lockFile(bool lock);
    virtual bool fileLocked();
//    virtual QString file();
    int fileSet() const;

    virtual int rowCount() const;
    virtual int columnCount() const;

    virtual const QString &getData(int row, int column) const;
    virtual QStringList getColumn(int column, int fromRow = 0) const;
    virtual const QStringList &getRow(int row) const;
    virtual int findRow(const QString& data, int column, int fromRow = 0) const;
    virtual int findExactRow(const QString& data, int column,
                             int fromRow = 0) const;
    virtual int findColumn(const QString& data, int row, int fromCol = 0) const;
    virtual int findExactColumn(const QString& data, int row,
                                int fromCol = 0) const;

    virtual const QString &getVersion() const;

    virtual void changeData(const QString &data, int row, int column);
    virtual int addRow(const QStringList &data);
    virtual bool rearrangeData(int row, int to);
    virtual bool removeRow(int row);


    virtual int addColumn(const QStringList &data);

    bool printCSV(bool markVersion = true);
    void printBackupCSV(const QString &extension, bool markVersion = true);


protected:
    QString &editData (int row, int column);
    QString errorMSG = "CSV ERROR";
    QStringList errorMSGL = QStringList(errorMSG);

private:
    bool parseCSV(QTextStream &text);
    QString outputText(QStringList &list) const;

private:
    QFile *_file;
    FileStatus _status;
    QStringList _emptyLine;
    QList<QStringList> _table;
    bool _changed;

    QString _loadedVersion;
    bool _firstPrint;
    bool _backup;
};

#endif // CSVPARSER_H
