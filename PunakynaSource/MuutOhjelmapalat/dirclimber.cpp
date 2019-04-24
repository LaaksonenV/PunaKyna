#include "dirclimber.h"

#include <QDir>
#include <QString>
#include <QRegularExpression>

DirClimber::DirClimber()
    : m_mountain(QDir())
    , m_level(QStringList())
    , m_files(QStringList())
    , m_route(-1)
    , m_nextlevel(nullptr)
{
}

DirClimber::DirClimber(QDir &base)
{
    m_mountain = base;
    m_level = base.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    m_files = base.entryList(QDir::Files);
    m_nextlevel = nullptr;
    m_route = -1;
    if (m_level.count())
    {
        base.cd(m_level.at(0));
        m_nextlevel = new DirClimber(base);
        m_route = 0;
    }
}

DirClimber::~DirClimber()
{
    if (m_nextlevel)
        delete m_nextlevel;
}

bool DirClimber::setBase(QDir &base)
{
    m_mountain = base;
    m_level = base.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    m_files = base.entryList(QDir::Files);
    m_route = -1;
    if (m_level.count())
    {
        base.cd(m_level.at(0));
        m_nextlevel = new DirClimber(base);
        m_route = 0;
        return true;
    }
    return false;
}


bool DirClimber::climb(QDir &mountain)
{
    mountain = m_mountain;
    if (m_nextlevel && m_route >= 0)
    {
        if (!m_nextlevel->climb(mountain))
        {
            mountain.cdUp();
            ++m_route;
            if (m_route >= m_level.count())
                return false;
            mountain.cd(m_level.at(m_route));
            m_nextlevel->setBase(mountain);

        }
        return true;
    }
    return false;
}

QString DirClimber::findLastfile(const QString &file)
{
    QString ret;
    if (m_nextlevel && m_route >= 0)
    {
        ret = m_nextlevel->findLastfile(file);
        if (!ret.isEmpty())
            return m_level.at(m_route) + ret;
    }

    if (m_files.filter(QRegularExpression(file)).count())
        return file;

    return QString();

}
