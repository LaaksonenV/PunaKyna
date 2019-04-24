#include "gradingelement.h"

#include <QGridLayout>
#include <QIntValidator>
#include <QCheckBox>
#include <QLineEdit>
#include <QStringList>
#include <QString>
#include <QPainter>
#include <QPushButton>

#include "settings.h"
#include "autocommenter.h"

GradingElement::GradingElement(Settings *set, unsigned id, QWidget *parent)
    : QWidget(parent)
    , _settings(set)
    , _id(id)
{
    QGridLayout *lay = new QGridLayout();

    _short = new QLineEdit(this);
    _short->setMaxLength(1);
    _short->setFixedWidth(15);
    _short->setPlaceholderText(tr("Pikanäppäin"));
    _short->setToolTip(tr("Painamalla tähän kenttään syötettyä näppäintä,"
                          " viereinen ruutu vaihtaa tilaansa"));
    lay->addWidget(_short,0,1,1,1);
    connect(_short, &QLineEdit::textEdited,
            this, [this](const QString & t){emit elementChanged(_id, t,
                                             QuestionFile::CommentRow_Id);});

    _name = new QLineEdit(this);
    _name->setPlaceholderText(tr("Arvostelukohdan nimi"));
    _name->setToolTip(tr("Nimeä käytetään vain tunnistukseen,\n"
                        "eikä se näy opiskelijalle.\n"
                        "Vain kommenttiriville kirjoitettu teksti\n"
                        "vaikuttaa ohjelman ulkopuolella"));
    lay->addWidget(_name,0,2,1,1);
    connect(_name, &QLineEdit::editingFinished,
            this, [this](){emit elementChanged(_id,
                                               _name->text(),
                                            QuestionFile::CommentRow_Label);});

    _check = new QCheckBox(this);
    connect(_check, &QCheckBox::toggled,
            this, &GradingElement::on_elementChecked);
    lay->addWidget(_check,0,0,1,1);

    _comment = new QLineEdit(this);
    _comment->setPlaceholderText(tr("Kommentti"));
    _comment->setToolTip(tr("Kirjoita tähän kommentti, jonka valittu \n"
                          "arvostelukohta antaa opiskelijan vastaukselle. \n"));
    lay->addWidget(_comment,1,0,1,5);
    connect(_comment, &QLineEdit::editingFinished,
            this, [this](){emit elementChanged(_id,
                                               _comment->text(),
                                           QuestionFile::CommentRow_Comment);});

    _points = new QLineEdit("0",this);
    _points->setValidator(new QIntValidator());
    _points->setFixedWidth(45);
    _points->setToolTip(tr("Arvostelukohdan antama pistemäärä.\n"
                        "Tämä voi olla myös negatiivinen luku."));
    lay->addWidget(_points,0,3,1,1);
    connect(_points, &QLineEdit::textEdited,
            this, [this](const QString & t){emit elementChanged(_id, t,
                                            QuestionFile::CommentRow_Points);});

    QPushButton *button = new QPushButton(QIcon(":/icons/auch"),
                                          QString(), this);
//    button->setFixedWidth(button->height());
    button->setToolTip(tr("Aseta kommentille avainsanat, joita automaatti"
                          "tarkastaja etsii tekstistä"));
    lay->addWidget(button,0,4);
    connect(button, &QPushButton::clicked,
            this, [this](){emit elementChanged(_id, "",
                                            QuestionFile::CommentRow_Keys);});


    setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::Fixed);
    lay->setContentsMargins(5,5,10,5);

    setLayout(lay);
}

void GradingElement::setContent(const QStringList &text)
{
    _short->setText(text.at(QuestionFile::CommentRow_Id));
    _name->setText(text.at(QuestionFile::CommentRow_Label));
    _comment->setText(text.at(QuestionFile::CommentRow_Comment));
    _points->setText(text.at(QuestionFile::CommentRow_Points));
    if (_points->text() == "")
        _points->setText("0");
}

void GradingElement::setShortcut(QString c)
{
    _short->setText(c);
}

bool GradingElement::checkElement(bool check)
{
    if ((check && _check->isChecked()) ||
        (!check && !_check->isChecked()))
         return false;
    else
        _check->setChecked(check);
    return true;
}

bool GradingElement::isElementChecked()
{
    return _check->isChecked();
}

void GradingElement::changeCheck()
{
    if (_check->isChecked())
        _check->setChecked(false);
    else
        _check->setChecked(true);
}

void GradingElement::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawRect(rect());
}

int GradingElement::getValue() const
{
    return _points->text().toInt();
}

QString GradingElement::getName() const
{
    return _name->text();
}

int GradingElement::on_elementChecked(bool checked)
{
    int i = _points->text().toShort();
    if (!checked)
        i = -i;
    emit elementChecked(_id, checked);
    return i;
}
