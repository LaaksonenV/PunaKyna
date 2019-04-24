#ifndef BROWSERMODELMOODLEAMB_H
#define BROWSERMODELMOODLEAMB_H

#include <QDialog>

class QButtonGroup;

class BrowserModelMoodleAmb : public QDialog
{
    Q_OBJECT
public:
    static int getCorrect(const QString &file,
                          const QStringList &possibles,
                          QWidget *parent = nullptr);

private:
    BrowserModelMoodleAmb(const QString &file,
                          const QStringList &possibles,
                          QWidget *parent = nullptr);
    int getSelected();
    QButtonGroup *_grp;

};

#endif // BROWSERMODELMOODLEAMB_H
