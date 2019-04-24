#include "smalldialogs.h"

#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QGridLayout>
#include <QBoxLayout>
#include <QSpinBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QComboBox>

#include "settings.h"

using namespace SmallDialogs;

void QuestionNumber::getNumbers(QMap<QString,int> &questions,
                                QVector<int> &points, QWidget *parent)
{
    QuestionNumber dialog(questions, points, parent);
    dialog.exec();
    QString key;
    for (int i = 0; i < questions.count(); ++i)
    {
        key = questions.keys().at(i);
        questions[key] = dialog.getNumber(key);
    }
    points = dialog.getPoints();
}


QuestionNumber::QuestionNumber(const QMap<QString, int> &questions,
                               QVector<int> &points,
                               QWidget *parent)
    : QDialog(parent)
    , _origin(questions)
    , _questions(questions.count())
    , _points(QVector<QSpinBox*>())
    , _selected(QVector<ushort>())
    , _totalPoints(new QLineEdit("6"))
    , _aliasList(QStringList())

    , _layout(new QGridLayout())
{
    foreach (QString s, questions.keys())
        _aliasList << s;

    QGridLayout *mainlay = new QGridLayout();

    QButtonGroup *radioGroup;
    QRadioButton *radio;

    QString key;
    _selected << 0;

    for (int i = 0; i < _origin.keys().count(); ++i)
    {
        key = _origin.keys().at(i);
        _layout->addWidget(new QLabel(key),
                           0 ,i+1,Qt::AlignBottom);
        radioGroup = new QButtonGroup;
        connect(radioGroup, SIGNAL(buttonToggled(int,bool)),
                this, SLOT(buttonToggled(int,bool)));
        _questions[i] = radioGroup;

        radio = new QRadioButton;
        radioGroup->addButton(radio, 0);
        radio->setChecked(true);
        _layout->addWidget(radio, 1, i+1);

    }
    _layout->addWidget(new QLabel(tr("T.0")),1,0);
    _layout->addWidget(new QLabel(tr("Pisteet")),0, _origin.keys().count()+1,
                       Qt::AlignBottom);

    QSpinBox *point = new QSpinBox(this);
    point->setMinimum(0);
    point->setValue(6);
    _points << point;

    connect(point, SIGNAL(valueChanged(int)), this, SLOT(pointsChanged()));
    _layout->addWidget(point, 1, _origin.keys().count()+1);

    int supQ = 0;
    int orQ;
    for (int i = 0; i < _origin.keys().count(); ++i)
    {
        key = _origin.keys().at(i);
        orQ = _origin.value(key);
        while (supQ < orQ)
        {
            addNumber();
            ++supQ;
        }
        _questions.at(i)->button(orQ)->setChecked(true);
        if (_points.count() > i && points.count() > i)
            _points.at(i)->setValue(points.at(i));
        connect(_questions.at(i), static_cast<void(QButtonGroup::*)(int)>
                (&QButtonGroup::buttonClicked),
                [=](int id){_origin[_origin.keys().at(i)] = id;});

    }

    QWidget *wi = new QWidget(this);
    wi->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    _layout->setVerticalSpacing(4);
    _layout->setMargin(0);
    _layout->setSizeConstraint(QLayout::SetMinimumSize);
    mainlay->addWidget(wi,0,0,1,4);
    mainlay->setColumnStretch(1,2);
    mainlay->setRowStretch(0,2);
    mainlay->setSizeConstraint(QLayout::SetMinimumSize);
    wi->setLayout(_layout);

    QPushButton *button = new QPushButton("+");
    mainlay->addWidget(button, 1, 0);
    connect(button, &QPushButton::clicked,
            this, &QuestionNumber::addNumber);

    mainlay->addWidget(new QLabel(tr("Yht: ")),1,1, Qt::AlignRight);
    QIntValidator *intval = new QIntValidator();
    intval->setBottom(0);
    _totalPoints->setValidator(intval);
    _totalPoints->setFixedWidth(30);
    mainlay->addWidget(_totalPoints,1,2, Qt::AlignLeft);

    QLabel *info = new QLabel(tr("Aseta kysymyksien tunnistetta vastaava "
                                 "tentin tehtävänumero ja pisteet. Mikäli "
                                 "tentti koostuu vain yhdestä kysymyksesta, "
                                 "jätä jokainen kysymys numeroon 0. Lisää "
                                 "mahdollisia tehtävänumeroita saa painamalla "
                                 "\"+\" painiketta."));
    info->setWordWrap(true);
    info->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    mainlay->addWidget(info, 3, 0, 1, 4);

    button = new QPushButton("Ok");
    button->setDefault(true);
    connect(button, &QPushButton::clicked,
            this, &QDialog::accept);

    mainlay->addWidget(button, 4, 2);

    setLayout(mainlay);
}

int QuestionNumber::getNumber(QString of)
{
    return _origin.value(of);
}

QVector<int> QuestionNumber::getPoints()
{
    QVector<int> ret;
    foreach (QSpinBox *s, _points)
    {
        ret << s->value();
    }
    ret << _totalPoints->text().toInt();
    return ret;
}

void QuestionNumber::addNumber()
{
    int row = _layout->rowCount();

    _layout->addWidget(new QLabel(tr("T.") + QString::number(row-1)), row, 0);
    QRadioButton *radio;
    for (int i = 0; i < _questions.count(); ++i)
    {

        radio = new QRadioButton;
        _questions[i]->addButton(radio, row-1);
        _layout->addWidget(radio, row, i+1);
    }

    QSpinBox *point = new QSpinBox(this);
    point->setMinimum(0);
    _points << point;

    connect(point, SIGNAL(valueChanged(int)), this, SLOT(pointsChanged()));
    _layout->addWidget(point, row, _origin.keys().count()+1);

    point->setValue(6);

    _selected << 0;

}

void QuestionNumber::pointsChanged()
{
    int sum = 0;
    foreach (QSpinBox *s, _points)
    {
        sum += s->value();
    }
    _totalPoints->setText(QString::number(sum));
}

void QuestionNumber::buttonToggled(int id, bool checked)
{
    if (checked)
        ++_selected[id];
    else
    {
        --_selected[id];
        if (_selected[id] <= 0)
            _points.value(id)->setValue(0);
    }
}

// // // // //

ExamType::ExamType(QStringList types, QWidget *parent)
    : QDialog(parent)
    , _dirType(new QComboBox())
    , _noFormat(new QRadioButton(tr("Vapaa")))
    , _selects(QVector<bool>())
    , _formats(QVector<QString>())
    , _checks(0)
{
    QBoxLayout *mainlay = new QBoxLayout(QBoxLayout::LeftToRight);
    QBoxLayout *seclay;
//    QRadioButton *radio;
    QCheckBox *chk;

    seclay = new QBoxLayout(QBoxLayout::TopToBottom);
    seclay->addWidget(new QLabel(tr("Tentin tyyppi:")), 0,nullptr);

/*
    radio = new QRadioButton(tr("EXAM"));
    _dirType->addButton(radio,0);
    radio->setChecked(true);
    seclay->addWidget(radio);

    radio = new QRadioButton(tr("Moodle"));
    _dirType->addButton(radio,1);
    seclay->addWidget(radio);
*/

    _dirType->addItems(types);
    _dirType->setMinimumContentsLength(20);
    _dirType->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    seclay->addWidget(_dirType);


    QPushButton *button = new QPushButton("Peruuta");
    connect(button, &QPushButton::clicked,
            this, &QDialog::reject);
    seclay->addWidget(button, 0, Qt::AlignBottom);

    mainlay->addLayout(seclay);
    mainlay->addSpacing(20);

    seclay = new QBoxLayout(QBoxLayout::TopToBottom);
    seclay->addWidget(new QLabel(tr("Sallitut formaatit:")), 0,nullptr);
    
    _noFormat->setChecked(true);
    seclay->addWidget(_noFormat);

    chk = new QCheckBox(tr("Pdf"));
    addToGroup(chk, "pdf");
    seclay->addWidget(chk);
    
    chk = new QCheckBox(tr("Html"));
    addToGroup(chk, "html?");
    seclay->addWidget(chk);

/*    chk = new QCheckBox(tr("MathCheck Html"));
    addToGroup(chk);
    seclay->addWidget(chk);*/

    button = new QPushButton("Ok");
    button->setDefault(true);
    connect(button, &QPushButton::clicked,
            this, &QDialog::accept);
    seclay->addWidget(button, 0, Qt::AlignBottom);

    mainlay->addLayout(seclay);
    setLayout(mainlay);
}

QString ExamType::getDirType() const
{
    return _dirType->currentText();
}

QStringList ExamType::getFileExtension() const
{
    QStringList ret;
    if(_checks <= 0)
        return ret;

    for (int i = 0; i < _selects.count(); ++i)
        if (_selects.at(i))
            ret << _formats.at(i);

    return ret;
}

void ExamType::addToGroup(QCheckBox *b, QString text)
{
    connect(_noFormat, &QRadioButton::clicked,
            [=](){b->setChecked(false);});
    int i = _selects.count();
    _selects.append(false);
    _formats.append(text);
    connect(b, &QCheckBox::clicked,
            [&,i](bool b){check(i,b);});
}

void ExamType::check(int i, bool check)
{
    _selects[i] = check;
    if (check)
    {
        ++_checks;
        _noFormat->setChecked(false);
    }
    else
    {
        --_checks;
        if (_checks == 0)
            _noFormat->setChecked(true);
    }
}

// // // // //

int Grading::getGradings(QVector<int> &refer, QVector<int> &points,
                         QWidget *parent)
{
    Grading dial(refer,points, parent);
    return dial.exec();
}

Grading::Grading(QVector<int> &refer, QVector<int> &points, QWidget *parent)
    : QDialog(parent)
    , _refer(refer)
    , _points(points)
    , _grades(6)
    , _percents(6)
    , _allpoints(0)
{
    setWindowTitle(tr("Aseta arvosanojen alarajat"));
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom,this);

    while (_refer.count() < 5)
        _refer.append(0);
//    if (_refer.count() == 5)
  //      _refer.append(_ultimateMax+1);

    QBoxLayout *allGradeLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    QBoxLayout *singleGradeLayout =
            new QBoxLayout(QBoxLayout::TopToBottom);

    QLabel *grade;
    singleGradeLayout->addWidget(new QLabel(tr("Arvosana")));
    singleGradeLayout->addWidget(new QLabel(tr("Alaraja")));
    singleGradeLayout->addWidget(new QLabel(tr("Arvosanoja")),
                                 0, Qt::AlignVCenter);
    allGradeLayout->addLayout(singleGradeLayout,1);

    int lim = 0;
    int amount = 0;

    QSpinBox *gradeBox = nullptr;
    QSpinBox *previous = nullptr;

    foreach (int p, points)
        _allpoints += p;
    if (_allpoints == 0)
        _allpoints = 1;

    for (int i = 0; i < 6; ++i)
    {
        singleGradeLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        singleGradeLayout->addWidget(new QLabel(QString::number(i)));
        if (i > 0)
        {
            gradeBox = new QSpinBox();
            gradeBox->setValue(_refer.at(i-1));
            if (i < 5)
                gradeBox->setMaximum(_refer.at(i));
            gradeBox->setFrame(false);
            gradeBox->setFixedWidth(30);
            gradeBox->setFixedSize(40,40);
            gradeBox->setKeyboardTracking(false);
            if (previous)
            {
                gradeBox->setMinimum(previous->maximum());
                connect(gradeBox, static_cast<void (QSpinBox::*)(int)>
                        (&QSpinBox::valueChanged),
                        previous, &QSpinBox::setMaximum);
                connect(previous, static_cast<void (QSpinBox::*)(int)>
                        (&QSpinBox::valueChanged),
                        gradeBox, &QSpinBox::setMinimum);
                setTabOrder(previous,gradeBox);
            }
            connect(gradeBox, static_cast<void (QSpinBox::*)(int)>
                    (&QSpinBox::valueChanged),
                    [=](int val){on_limitChange(val,i);});
            previous = gradeBox;
            singleGradeLayout->addWidget(gradeBox);
        }
        else
        {
            grade = new QLabel("0");
            grade->setFixedSize(40,40);
            singleGradeLayout->addWidget(grade);

        }

        amount = 0;
        while ((i >= 5 || lim < _refer.at(i)) && lim < _points.count())
        {
            amount += _points.at(lim);
            ++lim;
        }
        grade = new QLabel(QString::number(amount));
        _grades[i] = grade;
        singleGradeLayout->addWidget(grade);
        grade = new QLabel(QString::number(100*amount/_allpoints, 'f', 0)
                            + QString("%"));
        _percents[i] = grade;
        singleGradeLayout->addWidget(grade);
        allGradeLayout->addLayout(singleGradeLayout);

    }

    singleGradeLayout = new QBoxLayout(QBoxLayout::TopToBottom);

    singleGradeLayout->addStretch(3);
    singleGradeLayout->addWidget(new QLabel("max:" +
                                        QString::number(_points.count()-1)),1);

    singleGradeLayout->addStretch(5);
    allGradeLayout->addLayout(singleGradeLayout);

    QWidget *gradesWidget = new QWidget();
    allGradeLayout->setSizeConstraint(QLayout::SetMinimumSize);
    gradesWidget->setLayout(allGradeLayout);
    gradesWidget->setSizePolicy(QSizePolicy::Minimum,
                                QSizePolicy::Minimum);

    layout->addWidget(gradesWidget);
    layout->addStretch(2);

    QLabel *info = new QLabel(tr("Arvosanan pisterajaa ei voi asettaa "
                                 "pienemmäksi kuin edellisen arvosanan "
                                 "raja on, eikä suuremmaksi kuin "
                                 "seuraavan arvosanan raja on. Arvosanan 5 "
                                 "pisterajan voi asettaa yli tentin "
                                 "maksimipisteiden."));
    info->setWordWrap(true);
    layout->addWidget(info);



    QBoxLayout *buttonLayout = new QBoxLayout(QBoxLayout::LeftToRight);
    buttonLayout->addStretch(2);

    QPushButton *button = new QPushButton(tr("Peruuta"));
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked,
            this, &QDialog::reject);

    button = new QPushButton(tr("OK"));
    button->setDefault(true);
    buttonLayout->addWidget(button);
    connect(button, &QPushButton::clicked,
            this, &QDialog::accept);

    layout->addLayout(buttonLayout);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Minimum,
                  QSizePolicy::Minimum);
//    setFixedSize(500,250);
}

void Grading::on_limitChange(int value, int by)
{
    int amount = 0;
    int lim = 0;
    if (by > 1)
        lim = _refer.at(by-2);
    while (lim < value && lim < _points.count())
    {
        amount += _points.at(lim);
        ++lim;
    }
    _grades.value(by-1)->setText(QString::number(amount));
    _percents.value(by-1)->setText(QString::number(
                                     100*amount/_allpoints, 'f', 0)
                                  + QString("%"));


    _refer.replace(by-1, value);
    amount = 0;
    lim = value;
    while ((by >= 5 || lim < _refer.at(by)) && lim < _points.count())
    {
        amount += _points.at(lim);
        ++lim;
    }
    _grades.value(by)->setText(QString::number(amount));
    _percents.value(by)->setText(QString::number(
                                       100*amount/_allpoints,'f', 0)
                                    + QString("%"));

}

// // // // //
#include <QMessageBox>
#include <QRegExp>

bool AutoControl::createControl(QString &ctrl, QStringList &labels,
                                QWidget *parent)
{
    AutoControl dial(labels, parent);
    if (!ctrl.isEmpty())
        dial.setPrev(QString(ctrl));
    if (dial.exec())
    {
        ctrl = dial.getControl();
        labels = dial.getNoControl();
        return true;
    }
    return false;
}

AutoControl::AutoControl(QStringList &labels, QWidget *parent)
    : QDialog(parent)
    , control(new QLineEdit())
    , labelcount(labels.count())
{
    QVBoxLayout *lay = new QVBoxLayout();

    lay->addWidget(control);

    QLabel *lbl;
    lay->addWidget(lbl = new QLabel(tr("Ryhmitä loogisilla"
                                 " operaattoreilla halutut kommentit: "
                                 "esim 1&(2>3)!4.")));

    lbl->setWordWrap(true);
    lay->addWidget(lbl = new QLabel(tr("Kommentit merkitään numeroilla, ja niiden "
                                 "totuusarvo on tosi, jos automaatti "
                                 "on antanut kommentin vastaukselle. Sama "
                                 "kommentti voi esiintyä useammin. Kommentit "
                                 "joita ei esiinny, tarkastetaan normaalisti")));
    lbl->setWordWrap(true);
    lay->addWidget(new QLabel(tr("& = ja")));
    lay->addWidget(new QLabel(tr("/ = tai")));
    lay->addWidget(new QLabel(tr("> = (ja) jos")));
    lay->addWidget(new QLabel(tr("! = (tai) jos ei")));
    lay->addWidget(lbl = new QLabel(tr("jos (jos ei) tarkistuttaa seuraavan "
                                 "kommentin vain jos edellinen annettiin "
                                 "(ei annettu)")));
    lbl->setWordWrap(true);

    lay->addWidget(lbl = new QLabel(tr("Jos ja jos eiv"
                                       "operaattorit luetaan suluttamalla "
                                       "effektiivisesti oikealta vasemmalle, "
                                       "ja ja tai operaattorit suluttamalla "
                                       "vasemmalta oikealle: 1>2&3!4/5 sama "
                                       "kuin 1>((2&3)!(4/5))")));
    lbl->setWordWrap(true);
    lay->addWidget(lbl = new QLabel(tr("Käytä omia sulkuja välttääksesi "
                                       "järjesysestä koituvat virheet!")));
    lbl->setWordWrap(true);

    for (int i = 0; i < labels.count(); ++i)
        lay->addWidget(new QLabel(QString::number(i+1) +
                                  QString(" = ") +
                                  labels.at(i)));

    lay->addWidget(lbl = new QLabel(tr("? = Ilmoita käyttäjälle, "
                                 "että vastaus on tarkistettava käsin. "
                                 "Käytetään rakenteessa kommentin paikalla, "
                                 "totuusarvona aina tosi.")));
    lbl->setWordWrap(true);

    QHBoxLayout *blay = new QHBoxLayout();

    QPushButton *but = new QPushButton(tr("OK"));
    connect(but, &QPushButton::clicked, this, &AutoControl::on_Ok);
    blay->addWidget(but);

    but = new QPushButton(tr("Peruuta"));
    connect(but, &QPushButton::clicked, this, &QDialog::reject);
    blay->addWidget(but);

    lay->addLayout(blay);
    setLayout(lay);
}

void AutoControl::on_Ok()
{
    if (checkControl())
        accept();
}

void AutoControl::setPrev(const QString &ctrl)
{
    control->setText(ctrl);
}

bool AutoControl::checkControl()
{
    QString text = control->text();

    if (text.count("(") > text.count(")"))
    {
        QMessageBox::information(this, tr("Sulku auki"),
                                 tr("Tekstissä on yimääräinen avaava sulku"));
        return false;
    }
    else if (text.count("(") < text.count(")"))
    {
        QMessageBox::information(this, tr("Roikkuva sulku"),
                                 tr("Tekstissä on ylimääräinen sulkeva sulku"));
        return false;
    }
    else
    {
        QRegExp re("\\)");
        int i = 0;
        while ((i = re.indexIn(text, i)) >= 0)
        {
            ++i;
            if (text.left(i).count("(") < text.left(i).count(")"))
            {
                QMessageBox::information(this, tr("Virheellinen sulutus"),
                                         tr("Tekstissä on sulkeva sulku ennen "
                                            "avaavaa"));
                return false;
            }
        }
        re.setPattern("([^()\\d&>!?/])");
        if (re.indexIn(text) >= 0)
        {
            QMessageBox::information(this, tr("Tuntematon merkki"),
                                     tr("Tekstissä on tuntematon merkki ")
                                     + re.cap(1));
            return false;

        }
        re.setPattern("([)\\d?][!>&/][(\\d?])");
        if (re.indexIn(text, i) >= 0)
        {
                QMessageBox::information(this, tr("Operaattorivirhe"),
                                         tr("Operaattorin molemmilla puolilla "
                                            "pitää olla kommentti tai joukko."));
                return false;
        }
        re.setPattern("(\\d+)");
        i = 0;
        while ((i = re.indexIn(text, i)) >= 0)
        {
            if (re.cap(1).toInt() > labelcount)
            {
                QMessageBox::information(this, tr("Tuntematon kommentti"),
                                         tr("Tekstissä on liian iso kommentin "
                                            "numero."));
                return false;
            }
            i += re.matchedLength();
        }
    }
    return true;
}

QString AutoControl::getControl()
{
    return control->text();
}

QStringList AutoControl::getNoControl()
{
    QString text = control->text();
    QStringList ret;
    QRegExp re;
    for (int i = 1; i < labelcount+1; ++i)
    {
        re.setPattern(QString::number(i) + "\\D");
        if (re.indexIn(text) < 0)
            ret << QString::number(i);
    }
    return ret;
}

GradeLog::GradeLog(QVector<int> datapoints, QWidget *parent)
    : QDialog(parent)
{
    QGridLayout *lay = new QGridLayout;

    lay->addWidget(new QLabel(tr("Tyhjiä palautuksia:")),0,0);
    lay->addWidget(new QLabel(tr("Virheellisiä palautuksia:")),1,0);
    lay->addWidget(new QLabel(tr("Arvosteltuja:")),2,0);
    lay->addWidget(new QLabel(tr("Arvostelematta:")),3,0);

    lay->addWidget(new QLabel(QString::number(datapoints.at(1))),0,1);
    lay->addWidget(new QLabel(QString::number(datapoints.at(2))),1,1);
    lay->addWidget(new QLabel(QString::number(datapoints.at(4))),2,1);
    lay->addWidget(new QLabel(QString::number(datapoints.at(5))),3,1);

    QPushButton *but = new QPushButton(tr("Tulosta loki *TBA*"));
    connect(but, &QPushButton::clicked, this, &GradeLog::accept);
    lay->addWidget(but,4,0,1,1);

    but = new QPushButton("Ok");
    connect(but, &QPushButton::clicked, this, &GradeLog::reject);
    lay->addWidget(but,4,1);

    setLayout(lay);

}
