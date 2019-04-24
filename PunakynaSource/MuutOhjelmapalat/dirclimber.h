#ifndef DIRCLIMBER_H
#define DIRCLIMBER_H

#include <QStringList>
class QString;
#include <QDir>

class DirClimber
{
public:
    DirClimber();
    DirClimber(QDir &base);
    ~DirClimber();
    bool setBase(QDir &base);

    bool climb(QDir &mountain);

    QString findLastfile(const QString &file);

private:
    QDir m_mountain;
    QStringList m_level;
    QStringList m_files;
    int m_route;
    DirClimber *m_nextlevel;
};

#endif // DIRCLIMBER_H
