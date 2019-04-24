#ifndef CONTROLLEDCSVPARSER_H
#define CONTROLLEDCSVPARSER_H

#include "csvparser.h"
#include "enumsconsts.h"

#include <QVector>

/*Controlled CSV Parser distinguishes different parts of
 * the parsed CSV, giving new interface for premade comments access, and diverting
 * the rest of the rows to their expected calls
 */
class ControlledCSVParser : public CSVParser
{
public:
    ControlledCSVParser();

    virtual bool setFile(const QString &file);

    virtual int commentCount();
    virtual const QStringList &getComment(int row);
    virtual bool changeComment(const QString &data, int row,
                               Grading::CommentTable column);
    virtual void addComment(const QStringList &data);

    virtual int rowCount();
    virtual int addRow(const QStringList &data);
    virtual QString getData(int row, int column);
    virtual QStringList getColumn(int column, int fromRow = 0);
    virtual const QStringList &getRow(int row);
    virtual void changeData(const QString &data, int row, int column);

    virtual bool assignComment(unsigned short id, int to);
     virtual QVector<unsigned short> getComments(int of);
private:
    const QString _commentEndMSG = "#EOC";

    void bookNewcomers(int newcomerCommentsStart);
    void changeCommentedId(int oldId, int newId, int fromRow);

    int _gradingsAt;
};

#endif // CONTROLLEDCSVPARSER_H
