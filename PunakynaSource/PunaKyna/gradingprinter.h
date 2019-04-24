#ifndef GRADINGPRINTER_H
#define GRADINGPRINTER_H

#include <QObject>

class Settings;
class GradingElementWindow;
class CSVParser;
class QStringList;

class GradingPrinter : public QObject
{
    Q_OBJECT
public:
    explicit GradingPrinter(Settings *settings,
                            CSVParser *file,
                            GradingElementWindow *from,
                            QObject *parent = nullptr);

    QStringList completeComments(const QString &answer,
                                 bool actual = true);
    void exportToGrading(QString fileName);

    QStringList lockComments(int qCol, int aRow, int lockState,
                             bool actual = true);

    void save(int row, int column, bool ac = false);

    void fillDataRow(QStringList &d);

signals:
    void stateChanged(const QString &question, const QString &answer,
                      int state);

private:
    void csvError();

    CSVParser *_examFile;
    GradingElementWindow *_element;

    Settings *_settings;
};

#endif // GRADINGPRINTER_H
