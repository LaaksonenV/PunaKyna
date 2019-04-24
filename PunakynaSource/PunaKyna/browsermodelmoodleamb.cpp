#include "browsermodelmoodleamb.h"

#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGridLayout>
#include <QPushButton>

int BrowserModelMoodleAmb::getCorrect(const QString &file,
                                      const QStringList &possibles,
                                      QWidget *parent)
{
    BrowserModelMoodleAmb dial(file, possibles, parent);
    if (dial.exec() == QDialog::Accepted)
        return dial.getSelected();
    return -1;
}

BrowserModelMoodleAmb::BrowserModelMoodleAmb(const QString &file,
                                             const QStringList &possibles,
                                             QWidget *parent)
    : QDialog(parent)
    , _grp(new QButtonGroup(this))
{
    setWindowTitle(tr("Yhdistä tiedosto opiskelijalle"));
    QGridLayout *lay = new QGridLayout(this);

    QLabel *label = new QLabel(file, this);
    lay->addWidget(label, 0,0,3,1);

    QRadioButton *rbutton;
    int ind = 0;
    foreach (QString s, possibles)
    {
        rbutton = new QRadioButton(s, this);
        _grp->addButton(rbutton, ind);
        lay->addWidget(rbutton, ind+1,0,3,1);
        if (ind == 0)
            rbutton->setChecked(true);
        ++ind;
    }

    QPushButton *button = new QPushButton(tr("OK"), this);
    lay->addWidget(button, ind+1, 1 );
    connect(button, &QPushButton::clicked,
            this, &QDialog::accept);

    button = new QPushButton(tr("Peruuta"), this);
    lay->addWidget(button, ind+1, 2 );
    connect(button, &QPushButton::clicked,
            this, &QDialog::reject);
}

int BrowserModelMoodleAmb::getSelected()
{
    return _grp->checkedId();
}
