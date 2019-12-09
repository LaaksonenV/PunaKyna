#include "gradingwindow.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QVector>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QKeyEvent>
#include <QStringList>
#include <QDateTime>
#include <QAction>
#include <QMenu>
#include <QMimeData>

//#include <QtXlsx>

#include "binaryparser.h"
#include "smalldialogs.h"
#include "settings.h"
#include "gradingelementwindow.h"
#include "csvparser.h"
#include "csvversionresolver.h"
#include "gradingprinter.h"

GradingWindow::GradingWindow(Settings *set, QWidget *parent)
    : QWidget(parent)
    , _settings(set)
    , _examFile(nullptr)
    , _element(new GradingElementWindow(set, this))
    , _printer(nullptr)
    , _lay(new QVBoxLayout(this))
    , _lockButton(new QPushButton(QIcon(":/icons/lck"),"",this))
    , _fileRow(-1)
    , _fileColumn(-1)
{
    setLayout(_lay);

    connect(_element, &GradingElementWindow::commentRemoved,
            this, &GradingWindow::removeComment);
    connect(_element, &GradingElementWindow::commentsMoved,
            this, &GradingWindow::moveComments);

    _lay->addWidget(_element,2);

    QHBoxLayout *buttonlayout = new QHBoxLayout();
    _lay->addLayout(buttonlayout);

    QPushButton *button = new QPushButton(tr("Edellinen"),this);
    button->setToolTip(tr("Tällä hetkellä hiirellä kansiopuusta valitseminen"
                          "ei toimi tällä napilla selaamisen kanssa."));
    connect(button, SIGNAL(clicked()), this, SIGNAL(prevAnswer()));
    buttonlayout->addWidget(button);

    _lockButton->setCheckable(true);
    connect(_lockButton, &QPushButton::toggled,
            this, &GradingWindow::lockCurrent);
    buttonlayout->addWidget(_lockButton);

    button = new QPushButton(tr("Seuraava"),this);
    connect(button, SIGNAL(clicked()), this, SIGNAL(nextAnswer()));
    buttonlayout->addWidget(button);

}

GradingWindow::~GradingWindow()
{
    if (_examFile)
    {
        blockSignals(true);
        saveWork();
        blockSignals(false);
        delete _examFile;
        _examFile = nullptr;
    }
}

QString GradingWindow::checkExamFile(const QString &file, bool *found)
{
    QString fileName = file;
    if (!file.endsWith(".csv"))
        fileName += ".csv";
    QString ex = searchFilePath(fileName, 'E', found);
    if (ex.isEmpty())
        return QString();

    return ex + QDir::separator() + fileName;
}

bool GradingWindow::setExamFile(const QString &filePath,
                                QMap<QString, int> questions)
{
    _settings->writeDebug("Ladataan tenttitiedosto");

    // Testissä vasta vain yksi tentti per ohjelman suoritus
    if (_examFile)
        return false;
    _examFile = new CSVParser(filePath);

    CSVVersionResolver vres(_settings);
    if (vres.resolve(_examFile, CSVVersionResolver::CSVType_Exam))
        checkQuestionNumbers();

    if (!_examFile->fileSet())
        return false;

    /*
     * Check examfile for already commented cells and inform browser
     */
    parseStatuses();

    QString qid, fid;
    QString path, filename;
    QDir dir;
    CSVParser *file;
    QStringList prevQs = _examFile->getRow(0);
    if (prevQs.count())
        prevQs.removeFirst();
    QVector<int> points;
    int poi;

    /*
     * Parse examfiles unique questionheaders and look for questionfiles
     */
    for (int col = 1; col < _examFile->columnCount(); ++col)
    {
        qid = _examFile->getData(0,col);
        poi = 0;
        if (qid.count("-"))
        {
            poi = qid.section('-',-1).toInt();
            qid = qid.section('-', 0,-2);
            questions[qid] = poi;
        }
        if (_element->contains(qid))
            continue;

        fid = qid;
        fid.append(".csv");

        if (path.isEmpty())
            path = searchFilePath(fid, 'Q');

        if (path.isEmpty())
            return false;

        dir.setCurrent(path);

        file = new CSVParser(dir.absoluteFilePath(fid));
 //       vres.resolve(file,CSVVersionResolver::CSVType_Question);

        _element->addFile(qid, file);
        while (points.size() <= poi)
            points << 0;
        if (!points.at(poi))
            points[poi] = _element->getMaxPoints(qid);
    }

    /*
     * If folder contains new questions, they need to be added
     */
    if(prevQs.count() < questions.count())
    {
        QStringList prevQssimpl;
        foreach (QString q, prevQs)
            prevQssimpl << q.section("-", 0,-2);

        SmallDialogs::QuestionNumber::getNumbers(questions, points);
        int qnmb;

        QStringList maxP = _examFile->getData(0,0).split(":");
        while (maxP.count() < 2)
            maxP << "";

        maxP.replace(1,QString::number(points.takeLast()));
        _examFile->changeData(maxP.join(":"),0,0);

        foreach (QString q, questions.keys())
        {
            qnmb = questions.value(q);
            qid = q + QString("-") + QString::number(qnmb);

            /*
             * If there were no questions with indexed name, they are added
             */
            if (!prevQssimpl.contains(q))
            {
                _examFile->addColumn(QStringList(qid));

                filename =  q + QString(".csv");

                if (path.isEmpty())
                    path = searchFilePath(filename, 'Q');

                if (path.isEmpty())
                    return false;

                dir.setCurrent(path);

                file = new CSVParser(dir.absoluteFilePath(filename));
                //        vres.resolve(file,CSVVersionResolver::CSVType_Question);

                //vanhan pisteen tarkistus
                file->changeData(QString::number(points.at(qnmb)), 0
                                 , QuestionFile::Meta_Points);

                _element->addFile(q, file);
            }
            /*
             * If there was already aquestion, but its number was changed when
             * asking the new questions's numbers, it must be changed in the
             * file
             */
            else if (!prevQs.contains(qid) &&
                     _examFile->findExactColumn(qid,0,1) < 0)
            {
                _examFile->changeData(qid,0,_examFile->findColumn(q,0,1));
            }
        }
        _examFile->printCSV();

    }

    _printer = new GradingPrinter(_settings, _examFile, _element, this);
    connect(_printer, &GradingPrinter::stateChanged,
            this, &GradingWindow::stateChanged);

    return true;
}

bool GradingWindow::generateExamFile(const QString &filePath,
                                     QMap<QString, int> questions)
{
    _settings->writeDebug("Luodaan tenttitiedosto");

    // Testissä vasta vain yksi tentti per ohjelman suoritus
    if (_examFile)
        return false;
    _examFile = new CSVParser(filePath);

    if (!_examFile->fileSet())
        return false;

    QVector<int> points;

    SmallDialogs::QuestionNumber::getNumbers(questions, points);

    _examFile->addRow(QStringList(QString(":")
                       + QString::number(points.takeLast())));

    QString qid, filename;
    int qnmb;
    QString path;
    QDir dir;
    CSVParser *file;

    for (int i = 0; i < questions.keys().count(); ++i)
    {
        qid = questions.keys().at(i);
        qnmb = questions.value(qid);

        _examFile->addColumn(QStringList(qid + QString("-")
                                         + QString::number(qnmb)));

        filename =  qid + QString(".csv");

        if (path.isEmpty())
            path = searchFilePath(filename, 'Q');

        if (path.isEmpty())
            return false;

        dir.setCurrent(path);

        file = new CSVParser(dir.absoluteFilePath(filename));
//        vres.resolve(file,CSVVersionResolver::CSVType_Question);

//vanhan pisteen tarkistus
        file->changeData(QString::number(points.at(qnmb)), 0
                         , QuestionFile::Meta_Points);

        _element->addFile(qid, file);
    }
    _examFile->printCSV();


    _printer = new GradingPrinter(_settings, _examFile, _element, this);
    connect(_printer, &GradingPrinter::stateChanged,
            this, &GradingWindow::stateChanged);

    return true;
}

QString GradingWindow::searchFilePath(const QString &file, QChar type, bool *found)
{
    QString ret;
    QDir dir(QDir::home());
    QStringList paths;
    if (type == 'E')
        paths = _settings->getMultiValue(Settings::Section_Folders,
                                         "examfiles");
    else if (type == 'Q')
        paths = _settings->getMultiValue(Settings::Section_Folders,
                                         "questionfiles");
    foreach (QString s, paths)
    {
        dir.setPath(s);
        if (!dir.exists())
        {
            paths.removeOne(s);
            if (type == 'E')
                _settings->setMultiValue(Settings::Section_Folders,
                                                 "examfiles", paths);
            else if (type == 'Q')
                _settings->setMultiValue(Settings::Section_Folders,
                                                 "questionfiles", paths);
        }
        else if (dir.exists(file))
        {
            if (found)
                *found = true;
            return dir.absolutePath();
        }

    }
    if (paths.count())
        dir.setPath(paths.at(0));

    if (found)
        *found = false;


    if (type == 'E')
    {
        ret = _settings->getValue(Settings::Section_Folders, "defaultexam");
        if (ret.isEmpty())
        {
        QFileDialog dial(nullptr, tr("Valitse tenttilokien kansio"),
                         dir.absolutePath());
        dial.setFileMode(QFileDialog::Directory);
        dial.setLabelText(QFileDialog::Accept, tr("Valitse tenttilokien"
                                                  " kansio"));
        if (dial.exec())
            ret = dial.selectedFiles().at(0);
        else ret.clear();
        }

    }
    else if (type == 'Q')
    {
        ret = _settings->getValue(Settings::Section_Folders, "defaultquestion");
        if (ret.isEmpty())
        {
        QFileDialog dial(nullptr, tr("Valitse kysymystiedostojen kansio"),
                         dir.absolutePath());
        dial.setFileMode(QFileDialog::Directory);
        dial.setLabelText(QFileDialog::Accept, tr("Valitse kysymystiedostojen"
                                                  " kansio"));
        if (dial.exec())
            ret = dial.selectedFiles().at(0);
        else ret.clear();
        }

    }
    if(!ret.isEmpty())
    {
        if (type == 'E')
            _settings->addMultiValue(Settings::Section_Folders,
                                     "examfiles", ret);

        else if (type == 'Q')
            _settings->addMultiValue(Settings::Section_Folders,
                                     "questionfiles", ret);
        dir.setPath(ret);
        if (dir.exists(file))
            if (found)
                *found = true;
        return ret;
    }
    return QString();
}

void GradingWindow::parseStatuses()
{
    QString comment;
    QStringList commentList;
    for (int col = 1; col < _examFile->columnCount(); ++col)
    {
        QString qId = _examFile->getData(0,col).section('-',0,-2);
        for (int row = 1; row < _examFile->rowCount(); ++row)
        {
            comment = _examFile->getData(row, col);
            if (comment != "")
            {
                commentList = comment.split(Printing::separator);
                _printer->fillDataRow(commentList);

                if (commentList.at(ExamFile::CommentCell_Status) == "")
                {
                    if (commentList.at(ExamFile::CommentCell_Ids) != ""
                        || commentList.at(ExamFile::CommentCell_Free) != "")
                        comment = GradingState::stateToChar
                                        (GradingState::Commented);
                    else
                        comment = GradingState::stateToChar
                                        (GradingState::UnTouched);

                    commentList.replace(ExamFile::CommentCell_Status, comment);
                    _examFile->changeData(commentList.join(Printing::separator),
                                          row, col);
                }
                else
                    comment = commentList.at(ExamFile::CommentCell_Status);

                emit stateChanged(qId, _examFile->getData(row, 0),
                                  GradingState::charToState(comment));
            }
        }
    }
}

void GradingWindow::removeComment(unsigned comment)
{
    QString comments;
    QStringList commentList;
    BinaryParser bpars;
    BinaryParser::bins npars(0);
    int i;
    for (int row = 1; row < _examFile->rowCount(); ++row)
    {
        npars = BinaryParser::bins(0);
        comments = _examFile->getData(row, _fileColumn);
        if (!comments.isEmpty())
        {
            commentList = comments.split(Printing::separator);
            _printer->fillDataRow(commentList);

            if (!commentList.at(ExamFile::CommentCell_Ids).isEmpty())
            {
                bpars.setBinary(commentList.at(ExamFile::CommentCell_Ids));

                while (bpars.takeNext(i))
                {
                    if (i < int(comment))
                        BinaryParser::addBinary(npars,i);
                    else if (i > int(comment))
                        BinaryParser::addBinary(npars,i-1);
                }

                commentList.replace(ExamFile::CommentCell_Ids,
                                    BinaryParser::parseBinary(npars));
                _examFile->changeData(commentList.join(Printing::separator),
                                      row, _fileColumn);
                if (row == _fileRow)
                    _element->checkComments(npars);
            }
        }
    }
}

void GradingWindow::moveComments(QVector<int> &newOrder)
{
    QString comment;
    QStringList commentList;
    BinaryParser bpars;
    BinaryParser::bins npars(0);
    int i;
    for (int row = 1; row < _examFile->rowCount(); ++row)
    {
        npars = BinaryParser::bins(0);
        comment = _examFile->getData(row, _fileColumn);
        if (comment != "")
        {
            commentList = comment.split(Printing::separator);
            _printer->fillDataRow(commentList);

            if (!commentList.at(ExamFile::CommentCell_Ids).isEmpty())
            {
                bpars.setBinary(commentList.at(ExamFile::CommentCell_Ids));

                while (bpars.takeNext(i))
                    BinaryParser::addBinary(npars,newOrder.at(i));

                commentList.replace(ExamFile::CommentCell_Ids,
                                    BinaryParser::parseBinary(npars));
                _examFile->changeData(commentList.join(Printing::separator),
                                      row, _fileColumn);
            }
        }
        if (row == _fileRow)
            _element->checkComments(npars);
    }
}

void GradingWindow::importExamFile(const QString &file){Q_UNUSED(file)}

bool GradingWindow::changeGradeable(const QString &question, const QString &answer)
{
    _settings->writeDebug("Vaihda vastaus");

    QStringList comm;

    if (_fileRow >= 0)
        clearGradeable();

    if (answer.isEmpty())
        _fileRow = -1;
    else
        _fileRow = _examFile->findRow(answer,0,1);
    if (question.isEmpty())
        _fileColumn = -1;
    else
        _fileColumn = _examFile->findColumn(question, 0, 1);

    if (!_element->changeQuestion(question))
        return false;

    if (_fileColumn < 0)
        return false;

    if (_fileRow < 0)
    {
        if (answer.isEmpty())
            return false;
        _fileRow = _examFile->addRow(QStringList(answer));
        _printer->fillDataRow(comm);
        comm.replace(ExamFile::CommentCell_Status,
                     GradingState::stateToChar(GradingState::Viewed));
        _examFile->changeData(comm.join(Printing::separator),
                              _fileRow, _fileColumn);
        emit stateChanged(question, answer, GradingState::Viewed);
    }
    else
    {

        comm = _examFile->getData(_fileRow, _fileColumn)
                .split(Printing::separator);
        BinaryParser::bins binary = BinaryParser::makeBinary(
                    comm.at(ExamFile::CommentCell_Ids));
        _element->checkComments(binary);

        if (comm.count() > ExamFile::CommentCell_Free)
            _element->setPersonalComment(
                        comm.at(ExamFile::CommentCell_Free));
        if (comm.count() > ExamFile::CommentCell_Points)
            _element->setPersonalPoints(
                        comm.at(ExamFile::CommentCell_Points));

        if (comm.count() > ExamFile::CommentCell_Status)
        {
            if (GradingState::charToState(
                        comm.at(ExamFile::CommentCell_Status))
                    >= GradingState::Locked)
                changeLocked(true);
            else
                changeLocked(false);
            if (GradingState::charToState(
                        comm.at(ExamFile::CommentCell_Status))
                    == GradingState::UnTouched)
            {
                comm.replace(ExamFile::CommentCell_Status,
                             GradingState::stateToChar(GradingState::Viewed));
                _examFile->changeData(comm.join(Printing::separator),
                                      _fileRow, _fileColumn);
                emit stateChanged(question, answer, GradingState::Viewed);
            }
        }
        else
        {
            _printer->fillDataRow(comm);
            comm.replace(ExamFile::CommentCell_Status,
                         GradingState::stateToChar(GradingState::Viewed));
            _examFile->changeData(comm.join(Printing::separator),
                                  _fileRow, _fileColumn);
            emit stateChanged(question, answer, GradingState::Viewed);
        }

    }

    return true;
}

void GradingWindow::badEntry(const QString &question, const QString &answer,
                             int type)
{
    if (question.isEmpty() || answer.isEmpty())
        return;

    if (type != GradingState::Wrong && type != GradingState::Empty)
        return;

    int fileRow = _examFile->findRow(answer,0,1);
    int fileColumn = _examFile->findColumn(question, 0, 1);

    if (fileColumn < 0)
        return;

    QStringList comm;


    if (fileRow < 0)
        fileRow = _examFile->addRow(QStringList(answer));

    else
        comm = _examFile->getData(fileRow, fileColumn)
                .split(Printing::separator);

    _printer->fillDataRow(comm);
    comm.replace(ExamFile::CommentCell_Status,
                 GradingState::stateToChar(type));
    _examFile->changeData(comm.join(Printing::separator),
                          fileRow, fileColumn);
}

void GradingWindow::clearGradeable()
{
#ifdef DEBUGG
    if (_fileRow > 0 && _fileColumn > 0)
        _printer->save(_fileRow, _fileColumn);
#else
    saveAllWork();
#endif
    if (_element)
    {
        _element->checkComments(0);
        _element->setPersonalComment("");
        _element->setPersonalPoints("0");
        changeLocked(false);
    }
}



void GradingWindow::changeLocked(bool lock)
{
    if (lock)
    {
        _lockButton->setIcon(QIcon(":/icons/ulck"));
    }
    else
        _lockButton->setIcon(QIcon(":/icons/lck"));
    _lockButton->blockSignals(true);
    _lockButton->setChecked(lock);
    _lockButton->blockSignals(false);
    _element->lockComments(lock);
}

void GradingWindow::lockCurrent(bool lock)
{
    if (!_element || !_examFile || !_printer || _fileRow < 0 || _fileColumn < 0)
        return;

    if (lock)
        _printer->lockComments(_fileColumn, _fileRow, GradingState::Locked);
    else
        _printer->lockComments(_fileColumn, _fileRow, GradingState::NullState);
    changeLocked(lock);
}

void GradingWindow::exportToGrading(QString fileName)
{
    if (_printer)
        _printer->exportToGrading(fileName);
}

bool GradingWindow::keyPress(const QString &key)
{
    return _element->keyPress(key);
}

void GradingWindow::checkQuestionNumbers()
{
    QMap<QString,int> questions;
    QStringList str = _examFile->getRow(0);
    QString q, qid;
    QVector<int> points;
    int poi;
    for (int i = 1; i < str.count(); ++i)
    {
        q = str.at(i);
        qid = q.section('-',0,-2);
        poi = q.section('-',-1).toInt();
        questions.insert(qid,poi);
        while (points.size() <= poi)
            points << 0;
        if (!points.at(poi))
            points[poi] = _element->getMaxPoints(qid);
    }


    SmallDialogs::QuestionNumber::getNumbers(questions, points);

    _examFile->changeData(str.at(0).section(":",0,0) + QString(":")
                          + QString::number(points.last()),0,0);

    QString key;
    int column;
    for (int i = 0; i < questions.count(); ++i)
    {
        key = questions.keys().at(i);
        column = _examFile->findColumn(key,0,1);
        _examFile->changeData(key
                              + '-'
                              + QString::number(questions.value(key)), 0,
                              column);
    }
}

void GradingWindow::saveWork()
{
    if (!_examFile->fileSet())
        // FILEDIALOG
        return;
//        _examFile->setFile("TEMP.csv");
    if (_fileRow > 0 && _fileColumn > 0)
        _printer->save(_fileRow, _fileColumn);
    _examFile->printCSV();
}

void GradingWindow::saveAllWork()
{
    _element->saveWork();

    saveWork();
}

void GradingWindow::autoCheckCurrent(const CSVParser &text)
{
    autoCheck(text, QStringList(), QStringList());
}

void GradingWindow::autoCheck(const CSVParser &text,
                              const QStringList &question,
                              const QStringList &answer)
{
    int row, column;
    if (answer.isEmpty())
    {
        if (_fileRow < 0)
            return;
        row = _fileRow;
    }
    else
        row = _examFile->findRow(answer.at(BrowserMetaData::Meta_IdName),0,1);

    if (question.isEmpty())
    {
        if (_fileColumn < 0)
            return;
        column = _fileColumn;
    }
    else
        column = _examFile->
                findColumn(question.at(BrowserMetaData::Meta_IdName),0,1);

    if (row < 0 || column < 0)
    {
        if (column < 0)
            column = _examFile->
              addColumn(QStringList(question.at(BrowserMetaData::Meta_IdName)));
        if (row < 0)
        {
            row = _examFile->
                   addRow(QStringList(answer.at(BrowserMetaData::Meta_IdName)));
        }
    }
    else
    {
        QStringList comm = _examFile->getData(row, column)
                .split(Printing::separator);
        if (comm.count() > ExamFile::CommentCell_Status &&
                GradingState::charToState(comm.at(ExamFile::CommentCell_Status))
                >= GradingState::Commented)
            return;
    }
    if (row == _fileRow)
        _element->autoCheck(text, answer, question, true);
    else
        _element->autoCheck(text, answer, question);

    _printer->save(row, column, true);
}

void GradingWindow::editCurrentAuto()
{
    _element->createControl();
}

void GradingWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls() &&
        e->mimeData()->urls().first().fileName().section('.',-1) == ".csv")
        e->accept();
    else
        e->ignore();
}

void GradingWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urllst = e->mimeData()->urls();
    QString file = urllst.first().toString();
    file.remove("file://");
    CSVParser pars(file);
// TEstit että on kysymys tiedosto
    _element->copyfile(pars);
}

void GradingWindow::contextMenuEvent(QContextMenuEvent *e)
{
    QAction *newAct = new QAction(tr("Kopioi kommenttitiedosto"), this);
    connect(newAct, &QAction::triggered, this, &GradingWindow::on_copyComments);

    QMenu menu(this);
    menu.addAction(newAct);
    menu.exec(e->globalPos());
}

void GradingWindow::on_copyComments()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Valitse kysymystiedosto"),
                                            QDir::homePath(), "(*.csv)");
    if (fileName.isEmpty())
        return;

    CSVParser pars(fileName);
// TEstit että on kysymys tiedosto
    _element->copyfile(pars);
}
