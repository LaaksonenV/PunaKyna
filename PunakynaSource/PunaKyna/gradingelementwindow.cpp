#include "gradingelementwindow.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QVector>
#include <QString>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QIntValidator>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QRegExp>

#include "csvparser.h"
#include "gradingelement.h"
#include "settings.h"
#include "binaryparser.h"
#include "autocommenter.h"
#include "commentediting.h"
#include "smalldialogs.h"


GradingElementWindow::FileStore::FileStore(CSVParser *fl)
    : file(fl)
    , widg(nullptr)
    , comms(QVector<GradingElement *>())
{
    if (!file)
        file = new CSVParser();
}

GradingElementWindow::FileStore::~FileStore()
{
    delete file;
    if (widg)
        delete widg;
}

GradingElementWindow::GradingElementWindow(Settings *set, QWidget *parent)
    : QWidget(parent)
    , _settings(set)
    , _fileFolder(QMap<QString, FileStore*>())
    , _current("")
    , _area(new QScrollArea(this))
    , _pointCounter(new QLabel("0", this))
    , _points(0)
    , _maxPoints(new QLineEdit("0"))
    , _divider(new QLineEdit(""))
    , _freeComment(new QLineEdit(""))
    , _freePoints(new QLineEdit("0"))
    , _commented(0)
    , _autoCommented(0)
    , _autoConflicting(false)
{


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(_area, 0, 0, 1, 5);
    mainLayout->setRowStretch(0,2);
    mainLayout->setColumnStretch(2,2);

    _freeComment->setPlaceholderText(tr("Henkilökohtainen kommentti"));
    mainLayout->addWidget(_freeComment, 1, 0, 1, 4);

    QIntValidator *intVal = new QIntValidator();
    _freePoints->setValidator(intVal);
    _freePoints->setFixedWidth(30);
    mainLayout->addWidget(_freePoints, 1, 4);
    connect(_freePoints, &QLineEdit::textEdited,
            this, &GradingElementWindow::on_freePointChange);


    QPushButton *button = new QPushButton(tr("Uusi kommentti"), this);
    mainLayout->addWidget(button, 2, 0, 1, 3);
    connect(button, &QPushButton::clicked,
            this, &GradingElementWindow::on_addElement);

    QHBoxLayout *pLay = new QHBoxLayout();

    _pointCounter->setAlignment(Qt::AlignRight);
    pLay->addWidget(_pointCounter);
    pLay->addWidget(new QLabel("/"));
    _maxPoints->setValidator(new QIntValidator());
    _maxPoints->setFixedWidth(30);
    _maxPoints->setToolTip(tr("Tehtävän maksimipistemäärä."));
    pLay->addWidget(_maxPoints);
    connect(_maxPoints, &QLineEdit::textEdited,
            this, &GradingElementWindow::on_maxPointsChange);
    pLay->addWidget(new QLabel("/"));
    intVal = new QIntValidator();
    intVal->setBottom(1);
    _divider->setValidator(intVal);
    _divider->setFixedWidth(15);
    _divider->setToolTip(tr("Tehtävän pisteiden jakaja, ei vaikuta"
                            " maksimiin."));
    pLay->addWidget(_divider);
    connect(_divider, &QLineEdit::textEdited,
            this, &GradingElementWindow::on_DividerChange);



    pLay->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(pLay, 2, 3, 1, 2);

    setLayout(mainLayout);
}

GradingElementWindow::~GradingElementWindow()
{
    saveWork();
    foreach (FileStore *p, _fileFolder)
    {
        delete p;
    }
}

bool GradingElementWindow::addFile(const QString &ident, CSVParser *file)
{
    if (contains(ident))
        return false;

    _fileFolder.insert(ident, new FileStore(file));
    return true;
}

void GradingElementWindow::copyfile(CSVParser &file, QString to)
{
    if (to.isEmpty())
        to = _current;
    if (!_fileFolder.contains(to))
        return;

    FileStore *store = _fileFolder[to];
    CSVParser *toFile = store->file;

    int i = 0;
    if (toFile->rowCount())
    {
        i = 1;
        toFile->changeData(file.getData(0,0),0,0);
    }

    while (i < file.rowCount())
    {
// conflicresolveriin

        toFile->addRow(file.getRow(i));
        ++i;
    }


    if (toFile == _fileFolder.value(_current)->file)
    {
        if (_area->widget())
            delete _area->takeWidget();
        importComments(store);
    }
    else
    {
        delete store->widg;
        store->widg = nullptr;
    }

}

bool GradingElementWindow::contains(const QString &ident)
{
    if (_fileFolder.contains(ident))
        return true;
    return false;
}

bool GradingElementWindow::changeQuestion(const QString &ident)
{
    _autoConflicting = false;
    if (ident.isEmpty())
    {
        if (_area->widget() && !_current.isEmpty())
            _fileFolder.value(_current)->widg = _area->takeWidget();
        _current.clear();
    }
    if (_current == ident)
    {
        _commented = 0;
        checkComments(0);
        return true;
    }
    else if (_fileFolder.contains(ident))
    {
        importComments(_fileFolder.value(ident));
        _current = ident;
        return true;
    }
    return false;
}

bool GradingElementWindow::checkComments(BinaryParser::bins binary)
{
    blockSignals(true);
    if (_current.isEmpty() || !_fileFolder.contains(_current))
    {
        _settings->writeError("check comments, no current");
        return false;
    }
    foreach (GradingElement *g, _fileFolder.value(_current)->comms)
        g->checkElement(false);
    blockSignals(false);
    BinaryParser pars(binary);
    int i = pars.getNext();
    while (i >= 0)
    {
        _fileFolder.value(_current)->comms.at(i)->checkElement(true);
        i = pars.getNext();
    }
    return true;
}

void GradingElementWindow::setPersonalComment(const QString &text)
{
    _freeComment->setText(text);
}

void GradingElementWindow::setPersonalPoints(const QString &points)
{
    _freePoints->setText(points);
}

bool GradingElementWindow::lockComments(bool lock)
{
    _area->setDisabled(lock);
    _freeComment->setDisabled(lock);
    _freePoints->setDisabled(lock);
    return true;
}

void GradingElementWindow::createControl()
{
    FileStore *fs;

        if (_current.isEmpty())
            return;
        fs = _fileFolder.value(_current);

    QString control = fs->file->getData(0, QuestionFile::Meta_AutoControl);
    QStringList labels =  fs->file->getColumn(QuestionFile::CommentRow_Label,1);
    if (SmallDialogs::AutoControl::createControl(control, labels))
    {
        fs->file->changeData(control,0,QuestionFile::Meta_AutoControl);
        fs->file->changeData(labels.join("&"),0,QuestionFile::Meta_NoControl);
    }
}

void GradingElementWindow::autoCheck(const CSVParser &text,
                                     const QStringList &answer,
                                     const QStringList &question,
                                     bool curr)
{
    //No check for files metacolumns existence

    FileStore *fs;
    if (question.at(BrowserMetaData::Meta_IdName).isEmpty())
    {
        if (_current.isEmpty())
            return;
        fs = _fileFolder.value(_current);
    }
    else
        fs = _fileFolder.value(question.at(BrowserMetaData::Meta_IdName));

    _autoCommented = 0;
    _autoConflicting = false;

    QString control = fs->file->getData(0,QuestionFile::Meta_AutoControl);

    if (!control.isEmpty())
    {
        QVector<bool> checked(fs->file->rowCount()-1, false);
        QVector<bool> check(fs->file->rowCount()-1, false);
        QRegExp re("(\\d+)");

        Checker is;

        int ic = 0, i = 0;
        int skipping = 0;
        QChar c;
        while (ic < control.size())
        {
            c = control.at(ic);
            if (c.isDigit())
            {
                re.indexIn(control, ic);
                ic += re.matchedLength()-1;
                if (skipping)
                {
                    ++ic;
                    continue;
                }
                i = re.cap(1).toInt()-1;
                if (!checked.at(i))
                {
                    if (AutoCommenter::checkText(text,
                                             fs->file->getData(i+1,
                                                 QuestionFile::CommentRow_Keys),
                                             question, answer, true))
                    {
                        if ((question.at(BrowserMetaData::Meta_IdName).isEmpty()
                             || curr) &&
                                fs->comms.count() > i)
                            fs->comms.at(i)->checkElement(true);
                        BinaryParser::addBinary(_autoCommented, i);
                        is.truth = true;
                    }
                    else is.truth = false;
                    check[i] = is.truth;
                    checked[i] = true;
                }
                else is.truth = check.at(i);
                is.checkLine();
            }
            else if (c == '>')
            {
                if (!skipping && !is.truth)
                    ++skipping;

            }
            else if (c == '!')
            {
                if (!skipping && is.truth)
                    ++skipping;
            }
            else if (c == '&')
            {
                if (!skipping)
                    is.AND();
            }
            else if (c == '/')
            {
                if (!skipping)
                    is.OR();
            }
            else if (c == '(')
            {
                if (skipping)
                    ++skipping;
                else
                    is.newLine();
            }
            else if (c == ')')
            {
                if (skipping)
                    --skipping;
                else
                    is.endLine();
            }
            else if (c == '?')
            {
                if (!skipping)
                {
                    is.checkLine();
                    _autoConflicting = true;
                    is.truth = true;
                }
            }

            if (ic < 0)
                break;
            ++ic;
        }

/*        QStringList noControl =
                fs->file->getData(0, QuestionFile::Meta_NoControl).split("&");
        i = 0;
        foreach (QString s, noControl)
        {
            if (s.isEmpty()) {
                continue;
            }
            i = s.toInt()-1;
            if (AutoCommenter::checkText(text,
                                         fs->file->getData(i+1,
                                               QuestionFile::CommentRow_Keys),
                                         question, answer))
            {
                if ((question.at(BrowserMetaData::Meta_IdName).isEmpty()
                     || curr) && fs->comms.count() > i)
                    fs->comms.at(i)->checkElement(true);
                BinaryParser::addBinary(_autoCommented, i);
            }
        }*/
    }
    else for (int i = 0; i < fs->file->rowCount()-1; ++i)
    {
        if (AutoCommenter::checkText(text,
                                     fs->file->getData(i+1,
                                             QuestionFile::CommentRow_Keys),
                                     question, answer, false))
        {
            if ((question.at(BrowserMetaData::Meta_IdName).isEmpty()
                 || curr) && fs->comms.count() > i)
                fs->comms.at(i)->checkElement(true);
            BinaryParser::addBinary(_autoCommented, i);
        }
    }
}

GradingElementWindow::Checker::Checker()
    : truth(true)
    , truthAt(QVector<bool>(1,true))
    , ANDing(QVector<bool>(1,false))
    , ORing(QVector<bool>(1,false))
{}

void GradingElementWindow::Checker::newLine()
{
    truthAt << true;
    ANDing << false;
    ORing << false;
}
void GradingElementWindow::Checker::endLine()
{
    truthAt.takeLast();
    ANDing.takeLast();
    ORing.takeLast();
    checkLine();
}

void GradingElementWindow::Checker::checkLine()
{
    if (ANDing.last())
    {
        if (!truthAt.last())
            truth = false;
        ANDing.last() = false;
    }
    if (ORing.last())
    {
        if (truthAt.last())
            truth = true;
        ORing.last() = false;
    }
}

void GradingElementWindow::Checker::AND()
{
    truthAt.last() = truth;
    ANDing.last() = true;
}

void GradingElementWindow::Checker::OR()
{
    truthAt.last() = truth;
    ORing.last() = true;
}

void GradingElementWindow::saveWork()
{
    foreach (FileStore *p, _fileFolder.values())
    {
        p->file->printCSV();
    }
}

BinaryParser::bins GradingElementWindow::getCommented(bool autoc)
{
    if (autoc)
        return _autoCommented;
    return _commented;
}

bool GradingElementWindow::isConflicting()
{
    return _autoConflicting;
}


QString GradingElementWindow::getPersonalComment()
{
    return _freeComment->text();
}

QString GradingElementWindow::getPersonalPoints()
{
    return _freePoints->text();
}

QString GradingElementWindow::getCommentText(int comment, QString of)
{
    if (of.isEmpty())
        of = _current;
    if (!_fileFolder.contains(of))
        return QString();
    return _fileFolder.value(of)->file->getData(comment+1,
                                             QuestionFile::CommentRow_Comment);
}

int GradingElementWindow::getCommentValue(int comment, QString of)
{
    if (of.isEmpty())
        of = _current;
    if (!_fileFolder.contains(of))
        return -1;
    return _fileFolder.value(of)->file->getData(comment+1,
                                             QuestionFile::CommentRow_Points)
                                                .toInt();
}

int GradingElementWindow::getMaxPoints(QString of)
{
    if (of.isEmpty())
    {
        if (_current.isEmpty())
            of = _fileFolder.keys().first();
        else
            of = _current;
    }
    if (!_fileFolder.contains(of))
        return -1;
    return _fileFolder.value(of)->file->getData(0,QuestionFile::Meta_Points)
            .toInt();
}

int GradingElementWindow::getDivider(QString of)
{
    if (of.isEmpty())
    {
        if (_current.isEmpty())
            of = _fileFolder.keys().first();
        else
            of = _current;
    }
    if (!_fileFolder.contains(of))
        return 1;
    return _fileFolder.value(of)->file->getData(0,QuestionFile::Meta_Divider)
            .toInt();
}

bool GradingElementWindow::keyPress(const QString &key)
{
    if (_current.isEmpty() || !_fileFolder.contains(_current))
    {
        _settings->writeError("keypress, no current");
        return false;
    }
    int row = _fileFolder.value(_current)->file
            ->findRow(key, QuestionFile::CommentRow_Id, 1);
    if (row < 0)
        return false;
    _fileFolder.value(_current)->comms.at(row-1)->changeCheck();
    return true;
}

void GradingElementWindow::editComments()
{
    if(_current.isEmpty())
        return;

    FileStore *store = _fileFolder.value(_current);
    QStringList comments;
    for (int i = 0; i < store->comms.count(); ++i)
        comments << store->comms.at(i)->getName();

    CommentEditing edit(comments);
    if (edit.exec())
    {
        QVector<int> tbr = edit.getNewOrder();
        moveComments(tbr);
        tbr = edit.getRemoved();
        for (int i = tbr.count() -1; i > -1; --i)
            removeComment(tbr.at(i));
    }
}

void GradingElementWindow::removeComment(unsigned comment)
{
    if (_current.isEmpty())
        return;

    FileStore *store = _fileFolder.value(_current);
    store->file->removeRow(comment+1);
    store->comms.removeAt(comment);
    for (int i = comment; i < store->comms.count(); ++i)
        store->comms.at(i)->setId(i);

    if (_area->widget())
        delete _area->takeWidget();
    importComments(store);

    emit commentRemoved(comment);
}

void GradingElementWindow::moveComments(QVector<int> &newOrder)
{
    if (_current.isEmpty())
        return;

    FileStore *store = _fileFolder.value(_current);

    // by swapping
    QVector<int> newIndex;
    for (int i = 0; i < newOrder.count(); ++i) newIndex << i;
    for (int i = 0; i < newOrder.count()-1; ++i)
    {
        if (newOrder.at(i) == newIndex.at(i))
            continue;

        store->file->rearrangeData(newIndex.at(newOrder.at(i))+1, i+1);
        store->file->rearrangeData(i+2, newIndex.at(newOrder.at(i))+1);


        store->comms.move(newIndex.at(newOrder.at(i)), i);
        store->comms.move(i+1, newIndex.at(newOrder.at(i)));

        newIndex[i] = newOrder.at(i);
        newIndex[newOrder.at(i)] = i;

    }
    for (int i = 0; i < newOrder.count(); ++i) store->comms.at(i)->setId(i);

    if (_area->widget())
        delete _area->takeWidget();
    importComments(store);

    emit commentsMoved(newOrder);
}

GradingElement* GradingElementWindow::on_addElement()
{
    if (_current < nullptr || !_fileFolder.contains(_current))
    {
        _settings->writeError("on add element, no current");
        return nullptr;
    }
    FileStore *store = _fileFolder.value(_current);

// Binary parserille
    int iii = sizeof(BinaryParser::bins) * CHAR_BIT;
    if (store->comms.count() >= iii)
    {
        QMessageBox::critical(parentWidget(), tr("Liikaa kommentteja"),
          tr(QString("Tällä hetkellä " + QString::number(iii) +
             " on kommenttien määrän yläraja.\n"
             "Jos se ei riitä, sitä saadaan nostettua myöhemmissä versioissa.")
             .toStdString().c_str()),
                              QMessageBox::Ok);
        return nullptr;
    }
    QStringList text;
    fillDataRow(text);
    GradingElement *ret = addElement(store);

    bool sorted = false;
    for (int shrt = 0; shrt < 10; ++shrt)
        if (store->file->findRow(QString::number(shrt),0,1) < 0)
        {
            ret->setShortcut(QString::number(shrt));
            text[QuestionFile::CommentRow_Id] = QString::number(shrt);
            sorted = true;
            break;
        }
    if (!sorted)
    {
        QChar shrt = 'a';
        while (!sorted && shrt <= QChar('z'))
        {
            if (store->file->findRow(shrt,0,1) < 0)
            {
                ret->setShortcut(shrt);
                text[QuestionFile::CommentRow_Id] = shrt;
                sorted = true;
                break;
            }
            shrt = shrt.unicode() + 1;
        }
    }
    if (!sorted)
    {
        QChar shrt = 'A';
        while (!sorted && shrt <= QChar('Z'))
        {
            if (store->file->findRow(shrt,0,1) < 0)
            {
                ret->setShortcut(shrt);
                text[QuestionFile::CommentRow_Id] = shrt;
                break;
            }
            shrt = shrt.unicode() + 1;
        }
    }

    store->file->addRow(text);
    return ret;
}

short GradingElementWindow::on_elementCheck(unsigned i, bool check)
{
    if ((check && BinaryParser::checkBinary(_commented, i)) ||
        (!check && !BinaryParser::checkBinary(_commented, i)))
        // Jokin signaali ei ole tullut läpi, tai virhe importissa
        return 0;

    if (_current.isEmpty() || !_fileFolder.contains(_current))
    {
        _settings->writeError("element check, no current");
        return false;
    }
    int change = _fileFolder.value(_current)->comms.at(i)->getValue();
    if (!check)
        change = -change;
    _points += change;
    _pointCounter->setText(QString::number(_points +
                                           _freePoints->text().toInt()));

    BinaryParser::addBinary(_commented, i);

    emit pointsChanged(_points +
                       _freePoints->text().toInt());
    return change;
}

void GradingElementWindow::on_elementChange(unsigned i, QString newValue,
                               int column)
{
    if (_current.isEmpty())
        return;
    bool b = true;
    if (!newValue.isEmpty() && column == QuestionFile::CommentRow_Id)
    {
        int row = _fileFolder.value(_current)->file
                ->findRow(newValue, QuestionFile::CommentRow_Id , 1);
        if (row > 0)
        {
            //
            //MUUTA TOIMINTOA DUPLIKAATTI TILANTEESSA
            _fileFolder.value(_current)->comms.value(row-1)->setShortcut("");
            _fileFolder.value(_current)->file
                    ->changeData("", row, QuestionFile::CommentRow_Id);
        }
    }
    if (column == QuestionFile::CommentRow_Keys)
    {
        newValue = AutoCommenter::createKeywords(
                    _fileFolder.value(_current)->file->getData(i+1, column),
                    b, this);

    }
    if (column == QuestionFile::CommentRow_Points &&
        _fileFolder.value(_current)->comms.value(i)->isElementChecked())
    {
        _points = 0;
        foreach (GradingElement *e, _fileFolder.value(_current)->comms)
            if (e->isElementChecked())
                _points += e->getValue();
        _pointCounter->setText(QString::number(_points +
                                               _freePoints->text().toInt()));
    }

    if (b)
        _fileFolder.value(_current)->file->changeData(newValue, i+1, column);
}

int GradingElementWindow::on_maxPointsChange(const QString &points)
{
    if (_current.isEmpty())
        return 0;
    _fileFolder.value(_current)->file->changeData(points, 0,
                                                  QuestionFile::Meta_Points);
    return points.toInt();
}

int GradingElementWindow::on_DividerChange(const QString &points)
{
    if (_current.isEmpty())
        return 0;
    _fileFolder.value(_current)->file->changeData(points, 0,
                                                  QuestionFile::Meta_Divider);
    return points.toInt();
}

void GradingElementWindow::on_freePointChange(const QString &points)
{
    int po = points.toInt();
    if (points.isEmpty())
    {
        _freePoints->setText("0");
    }

    _pointCounter->setText(QString::number(_points + po));
    emit pointsChanged(_points + po);
}

void GradingElementWindow::importComments(FileStore *from)
{
    if (_area->widget() && !_current.isEmpty())
        _fileFolder.value(_current)->widg = _area->takeWidget();
    if (!from->file->rowCount())
        from->file->addRow(QStringList("6"));
    _maxPoints->setText(from->file->getData(0,QuestionFile::Meta_Points));
    _divider->setText(from->file->getData(0,QuestionFile::Meta_Divider));
    if (from->widg)
    {
        _area->setWidget(from->widg);
        _area->widget()->show();
        from->widg = nullptr;
        return;
    }

    from->comms.clear();
    generateNewLayout();
    GradingElement *element;
    for (int i = 1; i < from->file->rowCount(); ++i)
    {
        element = addElement(from);
        if (element == 0)
            return;
        element->blockSignals(true);
        element->setContent(from->file->getRow(i));
        element->blockSignals(false);
    }
}

void GradingElementWindow::generateNewLayout()
{
    QWidget *insider = new QWidget(this);
    insider->setSizePolicy(QSizePolicy::MinimumExpanding,
                           QSizePolicy::MinimumExpanding);

    QVBoxLayout *lay = new QVBoxLayout(insider);
    lay->setSizeConstraint(QLayout::SetNoConstraint);
    lay->setSpacing(5);
    lay->setContentsMargins(0,0,10,0);
    _area->setWidget(insider);
    _area->setWidgetResizable(true);
//    _area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _area->setSizePolicy(QSizePolicy::Ignored,
                         QSizePolicy::Ignored);

    insider->show();
}

GradingElement *GradingElementWindow::addElement(FileStore *store)
{
    int iii = sizeof(BinaryParser::bins) * CHAR_BIT;
    if (store->comms.count() >= iii)
    {
        QMessageBox::critical(parentWidget(), tr("Liikaa kommentteja"),
          tr("Ohjelmalle yritettiin antaa enemmän kommentteja kuin se sallii.\n"
             "Tällä hetkellä 64 on kommenttien määrän yläraja.\n"
             "Jos se ei riitä, sitä saadaan nostettua myöhemmissä versioissa."),
                              QMessageBox::Ok);
        return nullptr;
    }
    if (!_area->widget())
    {
        _settings->writeError("add element, no widget");
        return nullptr;
    }
    GradingElement *ret = new GradingElement(_settings, store->comms.count(),
                                             this);
    store->comms.append(ret);
    _area->widget()->layout()->addWidget(ret);
    _area->ensureWidgetVisible(ret);
    connect(ret, &GradingElement::elementChecked,
            this, &GradingElementWindow::on_elementCheck);
    connect(ret, &GradingElement::elementChanged,
            this, &GradingElementWindow::on_elementChange);


    ret->show();
    return ret;
}

void GradingElementWindow::fillDataRow(QStringList &d)
{
    for (int i = d.count(); i < QuestionFile::CommentRow_Size; ++i)
    {
        if (i == QuestionFile::CommentRow_Points)
            d << "0";
        else
            d << "";
    }
}

#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>

void GradingElementWindow::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(this);


    QAction *act = menu.addAction(tr("Siirrä tai poista kommennteja"),
                                  this, &GradingElementWindow::editComments);
    act->setStatusTip(tr("Kommenttien muokkaus vaikuttaa valittuihin "
                          "kommentteihin tenttitiedostoissa. Muokkaus "
                          "korjataan vain tämän hetkiseen tenttiin."));

    menu.addAction(tr("Muokkaa automaattitarkastajan toimintaa"),
                                 this, &GradingElementWindow::createControl);

    menu.exec(e->globalPos());
    e->accept();
}
