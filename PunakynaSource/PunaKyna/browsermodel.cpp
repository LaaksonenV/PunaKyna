#include "browsermodel.h"
#include "settings.h"
//#include "smalldialogs.h"
#include "csvparser.h"
#include "browsermodelmoodleamb.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QRegExp>
#include <QtXlsx>

#include <QtDebug>

/*!
 * \struct Indexer
 * \brief The Indexer struct acts as modelindex for the whole folder hierarchy
 *
 * Indexer contains integer values for the corresponding place in the models
 * datastructure.
 * If Indexing to top or middle of the structure, leave unneeded indexes as -1.
 *
 * Might be better to implement a sturdier indexing system, an iterator style
 * class mayhaps?
 */

/*!
 * \variable Indexer::quest
 * \brief Index of question
 */

/*!
 * \variable Indexer::answ
 * \brief Index of anwser
 */

/*!
 * \variable Indexer::xtr
 * \brief Index of extra, not currently used
 */

/*!
 * \class BrowserModel
 * \brief The BrowserModel class is the treemodel of the folderbrowser M/V
 * architecture
 *
 * To create a model of the folder, call setExamDir() with the parent
 * folder and identifier of the type of folder architecture.
 *
 * The model informs of changes to it via the dataChanged() signal
 */

/*!
 * \enum BrowserModel::DataType
 * \brief The DataType enum lists the types of data a branch may have
 * 
 * Lists indexes of directory structs parts for interaction with other
 * classes. New datatypes may be freely added before NullDS.
 * 
 * \value DisplayName  Text that should beshown to user
 * \value State  State of commenting of the answer, ie not seen, or 
 * autocommented
 * \value Identifier  Text for the internal identification, not shown to user
 * \value Url  Optional for a link to the answer on a web-page
 * \value File  Location of the answer file on computer
 * \value NullDS  Null value, MUST always be last value
 */

/*!
 * \enum BrowserModel::DirectoryTypes
 * \brief The DirectoryTypes enum tells the functions what kind of folder
 * structure to expect
 * 
 * TO BE REMOVED
 */

/*!
 * Constructs an empty model with settings \a set and \a parent.
 *
 * Remember to call BrowserModel::setExamDir() before trying to do anything else with the model
 */
BrowserModel::BrowserModel(Settings *set, QObject *parent)
    : QObject(parent)
    , _settings(set)
    , _infoName()
    , _top(nullptr)
    , _dirType(Type_Null)
    , _formats()
{
    QString dirtypeloc = _settings->getValue(Settings::Section_DEF,
                                             "directory_type_csv");

    // dirty default dirtype, maybe a better implementation somewhere else?
    QFile dirtype(dirtypeloc);
    if (!dirtype.exists() && dirtype.open(QFile::WriteOnly))
    {
        QTextStream str(&dirtype);
        str << "Name,ids,disp,url"
               "EXAM,\\(eid),,https://exam.tut.fi/exams/reviews/\\(eid)"
               ",\\(qid),,"
               ",\\(sid)-\\(aid),,https://exam.tut.fi/exams/review/\\(aid)"
               ",,,"
               "Moodle,\\(eid)-\\(qid),,"
               ",\\(sid)_\\(aid)_assign,,"
               ",,,"
               ",,,";
        dirtype.close();
    }

    _dirForm = new CSVParser(dirtypeloc);

    // Temporary fix for problems possibly due to folder permissions, just another dirty secondary implementation of default information
    if (!_dirForm->rowCount())
    {
        _dirForm->addRow(QStringList("Name")<<"ids"<<"disp"<<"url");
        _dirForm->addRow(QStringList("EXAM")<<"\\(eid)"<<""<<"https://exam.tut.fi/exams/reviews/\\(eid)");
        _dirForm->addRow(QStringList("")<<"\\(qid)"<<""<<"");
        _dirForm->addRow(QStringList("")<<"\\(sid)-\\(aid)"<<""<<"https://exam.tut.fi/exams/review/\\(aid)");
        _dirForm->addRow(QStringList("")<<""<<""<<"");
        _dirForm->addRow(QStringList("Moodle")<<"\\(eid)-\\(qid)"<<""<<"");
        _dirForm->addRow(QStringList("")<<"\\(sid)_\\(aid)_assign"<<""<<"");
        _dirForm->addRow(QStringList("")<<""<<""<<"");
        _dirForm->addRow(QStringList("")<<""<<""<<"");
    }
}

/*!
 * Destroys the BrowserModel object
 */
BrowserModel::~BrowserModel()
{
    if (_top)
        delete _top;
    if (_dirForm)
        delete _dirForm;
}

/*!
 * Sets a new model from directory \a folder, that is of \a type.
 * May be given an list of allowed file \a formats for the answers. Any answer
 * that is not of allowd format will be marked as such.
 * Currently may only be called once.
 *
 * Clears the existing model and tries to create a new one. If it can,
 * it will emit modelReady and return the name of its top folder
 */
QString BrowserModel::setExamDir(const QString &folder, DirectoryTypes type,
                                 QStringList formats)
{
    clear();

    QDir dir(folder);
    
    QStringList frm(formats);
    for (int i = 0; i < frm.count(); ++i)
    {
        frm[i].replace('|',"\\|");
    }

    QString format = frm.join('|');
    format.prepend('(');
    format.append(")$");
    _formats.setPattern(format);
    

    if (!populateDirectories(dir, type))
    {
        qWarning() << Q_FUNC_INFO << "populate failed";
        return QString();
    }

    _dirType = type;

    emit modelReady();
    return _top->idname;
}

QString BrowserModel::setExamDir(const QString &folder, QString type,
                                 QStringList formats)
{
    clear();

    QDir dir(folder);
    
    QStringList frm(formats);
    for (int i = 0; i < frm.count(); ++i)
    {
        frm[i].replace('|',"\\|");
    }

    QString format = frm.join('|');
    format.prepend('(');
    format.append(")$");
    _formats.setPattern(format);
    

    if (!populateDirectories(dir, type))
    {
        qWarning() << Q_FUNC_INFO << "populate failed";
        return QString();
    }

    emit modelReady();
    return _top->idname;
}

/*!
 * \brief BrowserModel::getQuestionNames Get the names of question folders
 * \return QMap of question name and number pairs
 *
 * If a model has been successfully set, this function will get the names of
 * question folders and, if possible, lookup for their number (EXAMs
 * summary), otherwise the paired number is 0
 */
QMap<QString, int> BrowserModel::getQuestionNames()
{
    QMap<QString, int> questionHeaders;

    if (!_top)
    {
        qWarning() << Q_FUNC_INFO << "called on empty model";
        return questionHeaders;
    }

    if (_dirType == Type_EXAM)
    {

        questionHeaders = searchQuestionNumbers();
        if (!questionHeaders.count())
            for (int i = 0; i < _top->unders.count(); ++i)
                questionHeaders.insert(_top->unders.at(i)->idname, 0);
    }

    else if (_dirType == Type_Moodle)
    {
        questionHeaders.insert(_top->idname,0);
    }

    else
    {
        for (int i = 0; i < _top->unders.count(); ++i)
            questionHeaders.insert(_top->unders.at(i)->idname, 0);
    }

    return questionHeaders;
}

QString BrowserModel::getGradings()
{
    QDir dir(_top->file);
    QStringList files;
    QString searchname = "null";
    if (_dirType == Type_Moodle)
    {
        searchname = _top->file.section(QDir::separator(),-1);
        searchname.prepend(tr("Grades-"));
    }
    else if (_dirType == Type_EXAM)
        searchname = "grading";

    searchname.append(".csv");

    do
    {
        files = dir.entryList(QDir::Files);
        if (files.contains(searchname))
            return dir.absoluteFilePath(searchname);

    } while (dir.cdUp());

    return QString();
}

/*!
 * \brief getText Get text of nodes data
 * \param of Indexer of the wanted node
 * \param type Wanted text slot
 * \return QString of text
 *
 * If index or type are invalid, will return an empty string
 */
QString BrowserModel::getText(Indexer of, BrowserModel::DataType type)
const
{
    Directory *cur = getDir(of);
    if (!cur)
    {
        _settings->writeError("Invalid index at getText");
        return "";
    }

    if (type == DisplayName)
        return cur->dispName;
    if (type == State)
        return QString::number(cur->state);
    if (type == Identifier)
        return cur->idname;
    if (type == Url)
        return cur->url;
    if (type == File)
        return cur->file;

    _settings->writeError("Invalid DataType at getText");
    return "";
}

QStringList BrowserModel::getMeta(Indexer of) const
{
    Directory *cur = getDir(of);
    QStringList ret = QStringList();
    while (ret.size() < BrowserMetaData::Meta_NULLData)
        ret << "";
    if (!cur)
    {
        _settings->writeError("Invalid index at getMeta");
    }
    else
    {
        ret[BrowserMetaData::Meta_DisplayName] = cur->dispName;
        ret[BrowserMetaData::Meta_IdName] = cur->idname;
    }
    return ret;
}

/*!
 * \brief setText Set text of a nodes data
 * \param of Indexer of the wanted node
 * \param type Wanted text slot
 * \param to String of text to set to
 * \return True if given slot can have given text, false otherwise
 *
 * Returns false if index is invalid, given type can't change its' text, or if
 * type is "State" and given text is not a state
 */
bool BrowserModel::setText(Indexer of, DataType type, const QString &to)
{
    Directory *cur = getDir(of);
    if (!cur)
    {
        _settings->writeError("Invalid index at setText");
        return false;
    }

    bool ret = true;
    if (type == DisplayName)
    {
        cur->dispName = to;
        emit dataChanged(of, type);
    }
    else if (type == State)
        ret = changeState(of, GradingState::charToState(to));

    else if (type == Url)
    {
        cur->url = to;
        emit dataChanged(of, type);
    }
    else
        ret = false;

    return ret;
}

/*!
 * \brief BrowserModel::getIndexOf Searches for the index of given names
 * \param question Name/Identifier of question
 * \param answer Optional name/identifier of answer (if applicable)
 * \return Indexer of question/answer
 *
 * if the given names are not found (answer under question), returns an empty
 * Indexer
 */
Indexer BrowserModel::getIndexOf(const QString &question, QString answer)
{
    Indexer ret;
    if (question.isEmpty())
        return ret;

    Directory *cur = nullptr;
    for (int i = 0; i < _top->unders.count(); ++i)
    {
        if (_top->unders.at(i)->idname == question)
        {
            cur = _top->unders.at(i);
            ret.quest = i;
            if (answer.isEmpty())
                return ret;
            for (int j = 0; j < cur->unders.count(); ++j)
            {
                if (cur->unders.at(j)->idname == answer)
                {
                    ret.answ = j;
                    return ret;
                }
            }
            return Indexer();
        }
    }
    return Indexer();
}

/*!
 * \brief BrowserModel::indexCount Gives the amount of subindexes
 * \param of top index
 * \return Amount of subindexes
 *
 * Expected to be obsolete an unused after new indexing system is implemented
 */
int BrowserModel::indexCount(Indexer of)
{
    Directory *cur = getDir(of);
    if (cur)
        return cur->unders.count();
    if (_top)
        return _top->unders.count();
    return 0;
}


//TODO
QStringList BrowserModel::getExamTypes()
{
    QStringList ret;
    if (_dirForm)
    {
        ret = _dirForm->getColumn(0,1);
        ret.removeAll("");
        ret.removeDuplicates();
    }
    return ret;
}

/*!
 * \brief BrowserModel::clear Clears the existing model
 * \internal
 */
void BrowserModel::clear()
{
    if (_top)
    {
        _top->destroy();
        delete _top;
        _top = nullptr;
    }
}

/*!
 * \brief BrowserModel::ensureDir Checks that given EXAM directory is viable
 * \param dir directory
 * \return \c true if directory is viable, \c false otherwise
 * \internal
 *
 * Checks that summary file can be found in or next to the given directory,
 * and moves it to the one with summary next to it.
 *
 * At the same time saves the location of the summaryfile
 */
bool BrowserModel::ensureDir(QDir &dir)
{


    // If given directory contains summary, the needed directory will be the
    // subdirectory.
    if (dir.exists("summary.txt"))
    {
        _infoName = dir.filePath("summary.txt");
        if (dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).count() == 1)
            dir.cd(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).at(0));
        else
        {
// Message to the user for folder error
            return false;
        }
    }
    else
    {
        QDir dir2(dir);
        dir2.cdUp();
        if (dir2.exists("summary.txt"))
            _infoName = dir2.filePath("summary.txt");
        else
            return false;
    }
    return true;
}

/*!
 * \brief BrowserModel::searchQuestionNumbers Reads summaryfile for question
 *  names and numbers
 * \return QMap of quiestion names and number pairs
 * \internal
 *
 * The search for question numbers is only an attempt, and if none can be found
 * (identifying text at the beginning of quiestion name), will mark it as 0
 *
 * Identifyintg text could be given by settings to make it more reliable and
 * robust by user. At the moment it is a QRegExp with a long and general
 * expression
 */
QMap<QString, int> BrowserModel::searchQuestionNumbers()
{
    QMap<QString, int> ret;
    QStringList questions;
    QFile infoFile(_infoName);
    if (infoFile.open(QFile::Text | QFile::ReadOnly))
    {
        QTextStream str(&infoFile);
        QString line;
        while (!str.atEnd())
        {
            line = str.readLine();
            if (line.startsWith("questions"))
                while (!str.atEnd())
                    questions << str.readLine();
        }
        infoFile.close();

        QString q,s;
        QRegExp rx("^\\s*(?:q|Q|t|T|Tehtävä)\\s*:?\\s*(\\d+)\\s+");
        for (int i = 0; i < questions.count(); ++i)
        {
            s = questions.at(i);
            q = s.section(":",0,0);
            s = s.section(":",1);

            if (rx.indexIn(s) >= 0)
            {
                _settings->writeDebug(q + ' ' + rx.cap(0));

                s = rx.cap(0);
            }
            else
                s = "0";
            ret.insert(q, s.toInt());
        }
    }
    return ret;
}

/*!
 * \brief BrowserModel::searchEXAMName reads summaryfile for exas name
 * \return QString of the name of the exam, or empty QString if it wasn't found
 * \internal
 */
QString BrowserModel::searchEXAMName()
{
    if (!_infoName.isEmpty())
    {
        QFile infoFile(_infoName);
        if (infoFile.open(QFile::Text | QFile::ReadOnly))
        {
            QTextStream str(&infoFile);
            QString line;
            while (!str.atEnd())
            {
                line = str.readLine();
                if (line.startsWith("exam name: "))
                    return line.remove(0,11);
            }
            infoFile.close();
        }
    }
    return QString();
}

/*!
 * \brief BrowserModel::populateDirectories Actually creates the model
 * \param dir QDir of the exam directory
 * \param type
 * \param format
 * \return \c true if no faults are encountered
 * \internal
 * \obsolete
 *
 * This function is a bit convoluted, and might benefit from being looked at
 *
 */
bool BrowserModel::populateDirectories(QDir &dir, DirectoryTypes type)
{
    // EXAMs directories have 2 possible directories that can be seen as the
    //  main directory.
    if (type == Type_EXAM && !ensureDir(dir))
    {
        qDebug() << Q_FUNC_INFO << "failed to ensure EXAM dir " << dir;
        return false;
    }

    _top = new Directory();

    _top->dispName = dir.dirName();
    _top->file = dir.absolutePath();
    _top->idname = dir.dirName();
    _top->state = GradingState::UnTouched;
    if (type == Type_EXAM)
    {
        _top->url = "https://exam.tut.fi/exams/reviews/";
        _top->url += dir.dirName();
        searchEXAMName();
    }

    QStringList entries = dir.entryList(QDir::AllEntries
                                        | QDir::NoDotAndDotDot);
    if (entries.isEmpty())
        return true;

    if (type == Type_EXAM)
        return populateExam(dir);
    else if (type == Type_Moodle)
        return populateMoodle(dir);

    return true;

}

bool BrowserModel::populateDirectories(QDir &dir, QString type)
{
    /*
    if (!ensureDir(dir))
    {
        qDebug() << Q_FUNC_INFO << "failed to ensure exam dir " << dir;
        return false;
    }
    */
    int row = 0;
    if (_dirForm)
    {
        row = _dirForm->findRow(type, 0, 1);
    }
    QMap<QString, Directory *> Qs;
    QStringList ids;
    for (int i = 0; i < 4; ++i)
        ids << "";
    
    return populateAll(dir,row,Qs,ids,row);
}

bool BrowserModel::populateAll(QDir &dir, int row,
                               QMap<QString, Directory *> &Qs,
                               QStringList &ids, int metaRow)
{

    QString pattern = _dirForm->getData(row,1);

    QStringList caplist;
    QRegularExpression caps("\\\\\\((.id)\\)");
    QRegularExpressionMatchIterator i = caps.globalMatch(pattern);
    while(i.hasNext())
    {
        QRegularExpressionMatch capm = i.next();
        caplist << capm.captured(1);
    }


    QRegularExpression relevantId(pattern.replace(
                                      QRegularExpression("\\\\\\(.id\\)")
                                                  ,"(.*)"));
    QRegularExpressionMatch relevant = relevantId.match(dir.dirName());
    
    if (caplist.count() == relevant.capturedTexts().count()-1)
    {
        for (int cap = 0; cap < caplist.count(); ++cap)
        {
            if (caplist.at(cap) == "eid")
            {
                ids[0] = relevant.captured(cap+1);
            }
            else if (caplist.at(cap) == "qid")
            {
                ids[1] = relevant.captured(cap+1);
            }
            else if (caplist.at(cap) == "aid")
            {
                ids[2] = relevant.captured(cap+1);
            }
            else if (caplist.at(cap) == "sid")
            {
                ids[3] = relevant.captured(cap+1);
            }
        }
    }
    else
    {
        // Lisää virheilmoitusta
        qDebug() << Q_FUNC_INFO << " no match";
    }

    QStringList entries;
    if (_dirForm->getData(row+1,1).isEmpty())
    {
        if (!_top)
        {
            _top = createBranch(metaRow, ids, 0);
        }
        Directory *Q;
        if (Qs.contains(ids.at(1)))
            Q = Qs.value(ids.at(1));
        else
        {
            Q = createBranch(metaRow, ids, 1);
            _top->unders.append(Q);
            Qs.insert(ids.at(1),Q);
        }

        Directory *A = createBranch(metaRow, ids, 2);
        Q->unders.append(A);
        A->file = dir.absolutePath();

        entries = dir.entryList(QDir::Files);
        if (entries.count() == 1)
        {
            A->file = dir.absoluteFilePath(entries.at(0));
            if(!checkFormat(entries.at(0)))
                A->state = GradingState::Wrong;
        }
        else if (!entries.count())   
        {
            A->state = GradingState::Empty;
        }
    }
    else
    {
        entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (QString entry, entries)
        {
            dir.cd(entry);
            populateAll(dir, row+1, Qs, ids, metaRow);
            dir.cdUp();
        }
    }
    return true;
}

BrowserModel::Directory* BrowserModel::createBranch(int metarow,
                                                   QStringList &ids,
                                                    int depth)
{
    Directory *ret = new Directory();

    QString meta = lookUpMeta(metarow+depth,2,ids);
    if (meta.isEmpty())
        meta = ids.value(depth);
    ret->dispName = meta;

    ret->idname = ids.value(depth);
    ret->state = GradingState::UnTouched;

    meta = lookUpMeta(metarow+depth,3,ids);
    ret->url = meta;

    return  ret;
}

QString BrowserModel::lookUpMeta(int row, int col, QStringList &ids)
{
    QString meta = _dirForm->getData(row,col);
    if (meta.isEmpty())
        return meta;

//    QRegularExpressionMatch caps = QRegularExpression("\\\\\\((.id)\\)")
  //          .match(meta);

    QStringList caplist;
    QRegularExpression caps("\\\\\\((.id)\\)");
    QRegularExpressionMatchIterator i = caps.globalMatch(meta);
    while(i.hasNext())
    {
        QRegularExpressionMatch capm = i.next();
        caplist << capm.captured(1);
    }

    for (int cap = 0; cap < caplist.count(); ++cap)
    {
        if (caplist.at(cap) == "eid")
        {
            meta.replace("\\(eid)", ids.value(0));
        }
        else if (caplist.at(cap) == "qid")
        {
            meta.replace("\\(qid)", ids.value(1));
        }
        else if (caplist.at(cap) == "aid")
        {
            meta.replace("\\(aid)", ids.value(2));
        }
        else if (caplist.at(cap) == "sid")
        {
            meta.replace("\\(sid)", ids.value(3));
        }

    }
    return meta;
}

bool BrowserModel::populateExam(QDir &dir)
{
    QStringList ansEntries;
    QStringList fileEntries;
    Directory *questionItem = nullptr;
    Directory *answerItem = nullptr;

    QRegularExpression relevantId("tut\\.fi.(\\d+).*(-\\d+)$");
    QRegularExpressionMatch relevant;
    QString answerId;
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);


    for (int i = 0; i < entries.count(); ++i)
    {
        dir.cd(entries.at(i));
        ansEntries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (ansEntries.isEmpty())
            continue;

        questionItem = new Directory();
        questionItem->dispName = dir.dirName();
        questionItem->idname = dir.dirName();
        fileEntries = dir.entryList(QDir::Files);
        if (fileEntries.count() < 0)
            questionItem->file = "";
        else if (fileEntries.count() >= 1)
            // Multiple templates?
            questionItem->file = dir.absoluteFilePath(
                        fileEntries.at(0));

        questionItem->state = GradingState::UnTouched;
        _top->unders.append(questionItem);
        for (int j = 0; j < ansEntries.count(); ++j)
        {
            dir.cd(ansEntries.at(j));
            fileEntries = dir.entryList(QDir::Files);
            answerItem = new Directory;

            answerId = dir.dirName();
            relevant = relevantId.match(answerId);
            if(relevant.hasMatch())
            {
                answerId = relevant.captured(1);
//                answerId.append('-');
                answerId.append(relevant.captured(2));
            }
            else
                qWarning("No Id found in folders");

            answerItem->dispName = answerId;
            answerItem->idname = answerId;
            answerItem->file = dir.absolutePath();
            answerItem->url = "https://exam.tut.fi/exams/review/";
            answerItem->url += dir.dirName().section("-",1);


            questionItem->unders.append(answerItem);
            if (fileEntries.count() == 1)
            {
                answerItem->file = dir.absoluteFilePath(
                            fileEntries.at(0));
                if(checkFormat(fileEntries.at(0)))
                {
                    answerItem->state = GradingState::UnTouched;
                }
                else
                    answerItem->state = GradingState::Wrong;


            }
            else if (!fileEntries.count())
                answerItem->state = GradingState::Empty;
            else
                answerItem->state = GradingState::UnTouched;
            // ELSE useampi tiedosto?

            dir.cdUp();
        }

        dir.cdUp();
    }
    return true;
}

bool BrowserModel::populateMoodle(QDir &dir)
{
    _top->dispName = dir.dirName().section('-',0,-2);
    Directory *questionItem = new Directory(_top);
    questionItem->dispName = dir.dirName().section('-',-1);

    QStringList fileEntries = dir.entryList(QDir::Files);
    if (fileEntries.count() < 0)
        questionItem->file = "";
    else if (fileEntries.count() >= 1)
        // Multiple templates?
        questionItem->file = dir.absoluteFilePath(
                    fileEntries.at(0));

    _top->unders.append(questionItem);

    Directory *answerItem = nullptr;
    QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QString line;
    for (int i = 0; i < entries.count(); ++i)
    {
        dir.cd(entries.at(i));

        fileEntries = dir.entryList(QDir::Files);
        answerItem = new Directory();
        line = dir.dirName();
        line.remove("_assignsubmission_file_");
        answerItem->dispName = line.section('_',0,-2);
        answerItem->idname = line.section('_',-1);

        if (fileEntries.count() == 1)
        {
            answerItem->file = dir.absoluteFilePath(
                        fileEntries.at(0));
            if(checkFormat(fileEntries.at(0)))
                answerItem->state = GradingState::UnTouched;
            else
                answerItem->state = GradingState::Wrong;
        }
        else if (!fileEntries.count())
            answerItem->state = GradingState::Empty;
        else
            answerItem->state = GradingState::UnTouched;
        // ELSE useampi tiedosto?

        questionItem->unders.append(answerItem);
        dir.cdUp();

    }
    return true;
}

bool BrowserModel::checkFormat(const QString &entry)
{
    if(_formats.pattern().isEmpty())
        return true;

    if (_formats.match(entry).hasMatch())
        return true;
    return false;
}

/*!
 * \brief BrowserModel::changeState Changes the state of node, and any node
 * over it if needed
 * \param of Index of node that changed
 * \param newState new state of the node
 * \return \c true, unless \a newState is invalid
 * \internal
 *
 * This will change the given nodes state to given state and emit an appropriate
 * signal, and then change tho higher nodes state to "lowest" state of its
 * branches. (i.e. if all other nodes are at state >"1", and given node is now
 * changed to "1", the higher node will also change to "1".)
 *
 * Function is a bit convoluted, and could possibly be made recursive for
 * slightly shorter code
 */
bool BrowserModel::changeState(Indexer of, int newState)
{
    if (newState >= GradingState::NullState)
        return false;
    Directory *cur = _top;
    int maxState = newState;
    int currentState = GradingState::NullState;
    Indexer copyOf(of);
    if (copyOf.quest >= 0)
    {
        cur = cur->unders.at(copyOf.quest);
        if (copyOf.answ >= 0)
        {
            // check every answer for the "lowest" state under the question
            for (int a = 0; a < cur->unders.count(); ++a)
            {
                currentState = cur->unders.at(a)->state;
                // Empty and Wrong states may be disregarded
                if (currentState == GradingState::Empty ||
                    currentState == GradingState::Wrong)
                    continue;
                // every other answer may be lower than the new, and only
                // the given index will change
                if (a != copyOf.answ)
                {
                    if (currentState != maxState)
                        maxState = (maxState < currentState
                                    ? maxState : currentState);
                }
                else
                    cur->unders.at(a)->state = newState;
            }
            emit dataChanged(copyOf, State);
            newState = maxState;
        }
        // now to check the "lowest" state of each question
        for (int q = 0; q < _top->unders.count(); ++q)
        {
            currentState = _top->unders.at(q)->state;
            // Empty and Wrong states may be disregarded
            if (currentState == GradingState::Empty ||
                currentState == GradingState::Wrong)
                continue;
            if (q != copyOf.quest)
            {
                if (currentState != maxState)
                    maxState = (maxState < currentState ? maxState : currentState);
            }
            else
                _top->unders.at(q)->state = newState;
        }
        copyOf.answ = -1;
        emit dataChanged(copyOf, State);
        newState = maxState;
    }
    _top->state = newState;
    copyOf.quest = -1;
    emit dataChanged(copyOf, State);
    return true;
}

/*!
 * \brief BrowserModel::getDir Returns the BrowserModel::Directory of the index
 * \param of Indexer of wanted directory
 * \return BrowserModel::Directory of given index, \c nullptr if index is
 * invalid
 * \internal
 */
BrowserModel::Directory *BrowserModel::getDir(Indexer of) const
{
    Directory *cur = _top;
    if (of.quest >= 0)
    {
        if (of.quest >= cur->unders.count())
            return nullptr;
        cur = cur->unders.at(of.quest);
        if (of.answ >= 0)
        {
            if (of.answ >= cur->unders.count())
                return nullptr;
            cur = cur->unders.at(of.answ);
        }
    }
    return cur;
}

/*!
 ' \struct BrowserModel::Directory
 * \brief The BrowserModel::Directory struct contains all data connected to a
 * node in the treemodel
 * \internal
 */

/*!
 * \brief BrowserModel::Directory::destroy Recursive destructor for the whole
 * tree
 * \internal
 */
void BrowserModel::Directory::destroy()
{
    foreach (Directory *d, unders)
    {
        d->destroy();
        delete d;
        d = nullptr;
    }
}
