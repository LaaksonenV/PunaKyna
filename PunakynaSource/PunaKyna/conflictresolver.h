#ifndef CONFLICTRESOLVER_H
#define CONFLICTRESOLVER_H

#include <QDialog>

template<typename T,typename U> class QMap;

#include "csvparser.h"

// konfliktiivisten tiedostojen yhdistämiseen, treeview original, list added, drag&drop tavstva
// listasta puuhun toppiin tai valmiin alle
class ConflictResolver : public QDialog
{
    Q_OBJECT

public:
    static void mergeExams(CSVParser &original, CSVParser &added);

    ConflictResolver(CSVParser &original, CSVParser &added, QWidget *parent = 0);

    int getMapped(int of);

private:
    bool different(QStringList row1, QStringList row2);

private:
    QMap<int, int> _mapping; //added -> original
};

#endif // CONFLICTRESOLVER_H
