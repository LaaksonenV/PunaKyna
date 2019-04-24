#include "commentediting.h"

#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>

#include <QPainter>
#include <QPushButton>

using namespace Editor;

Comment::Comment(QString name, int id, QWidget *parent)
    : QWidget(parent)
    , _name(name)
    , _id(id)
    , _selected(false)
    , _tbr(false)
{
    QPushButton *but = new QPushButton("X", this);
    but->resize(comment_height/2, comment_height/2);
    but->move(comment_height/4, comment_height/4);
    but->setCheckable(true);
    connect(but, &QPushButton::clicked, this, &Comment::remove);

    resize(sizeHint());
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

QSize Comment::sizeHint() const
{
    return QSize(window_def_length, comment_height);
}

void Comment::moveByStep(int i)
{
    move(0,pos().y()+comment_height*i);
}

void Comment::remove(bool rem)
{
    _tbr = rem;
    update();
}

bool Comment::toBeRemoved() const
{
    return _tbr;
}

void Comment::select(bool sel)
{
    _selected = sel;
    update();
}

int Comment::getId() const
{
    return _id;
}

void Comment::mousePressEvent(QMouseEvent *)
{
    select(!_selected);
    emit selected(this, _selected);
}

void Comment::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QBrush b;
    if (_tbr)
        b.setColor(Qt::gray);
    else
        b.setColor(Qt::white);
    p.setBrush(b);

    QPen pen;
    if (_selected)
        pen.setWidth(3);
    else
        pen.setWidth(1);
    p.setPen(pen);

    p.drawRect(rect());

    p.setBrush(QBrush());
//    p.setPen(pen);
    p.drawText(comment_height, comment_height/2-text_height/2, _name);
}

CommentWindow::CommentWindow(QWidget *parent)
    : QWidget(parent)
    , _selected(nullptr)
{
    setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
}

QSize CommentWindow::sizeHint() const
{
    return QSize(window_def_length, _comments.count()*comment_height);
}

void CommentWindow::addComment(QString name)
{
    Comment *com = new Comment(name, _comments.count(), this);
    connect(com, &Comment::selected, this, &CommentWindow::on_comment_select);
    com->moveByStep(_comments.count());
    _comments.append(com);
    resize(sizeHint());
}

QVector<int> CommentWindow::getRemoved()
{
    QVector<int> ret;
    for(int i = 0; i < _comments.count(); ++i)
    {
        if (_comments.at(i)->toBeRemoved())
            ret.append(i);
    }
    return ret;
}

QVector<int> CommentWindow::getNewOrder()
{
    QVector<int> ret;
    Comment *c;
    for(int i = 0; i < _comments.count(); ++i)
    {
        c = _comments.at(i);
        ret.append(c->getId());
    }
    return ret;
}

void CommentWindow::moveComment(bool up)
{
    if (!_selected)
        return;

    int row = _comments.indexOf(_selected);
    int direction;
    if (!up)
        direction = 1;
    else
        direction = -1;

    _selected->moveByStep(direction);

    Comment *dodge = _comments.at(row+direction);
    dodge->moveByStep(-direction);

    _comments.swap(row+direction, row);
}


void CommentWindow::paintEvent(QPaintEvent *)
{
}

void CommentWindow::resizeEvent(QResizeEvent *e)
{
    if (e->oldSize().width() != width())
        foreach (Comment *c, _comments)
            c->resize(width(), comment_height);
}

void CommentWindow::on_comment_select(Comment *c, bool sel)
{
    if (sel)
    {
        if (_selected)
            _selected->select(false);

        _selected = c;
    }
    else if (_selected && _selected == c)
        _selected = nullptr;
}

#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>

CommentEditing::CommentEditing(const QStringList &comments, QWidget *parent)
    : QDialog(parent)
    , wind(new CommentWindow(this))
{
    QGridLayout *lay = new QGridLayout(this);

    QScrollArea *area = new QScrollArea(this);

    for (int i = 0; i < comments.count(); ++i)
        wind->addComment(comments.at(i));

    area->setWidget(wind);

    lay->addWidget(area,0,0,4,2);

    QPushButton *but = new QPushButton("/\\", this);
    but->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(but, &QPushButton::clicked, [=](){wind->moveComment(true);});
    lay->addWidget(but,1,2);

    but = new QPushButton("\\/", this);
    but->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    connect(but, &QPushButton::clicked, [=](){wind->moveComment(false);});
    lay->addWidget(but,2,2);

    but = new QPushButton(tr("Ok"), this);
    connect(but, &QPushButton::clicked, this, &QDialog::accept);
    lay->addWidget(but,4,1);

    but = new QPushButton(tr("Peruuta"), this);
    connect(but, &QPushButton::clicked, this, &QDialog::reject);
    lay->addWidget(but,4,2);
    but->setDefault(true);
}

QVector<int> CommentEditing::getRemoved()
{
    return wind->getRemoved();
}

QVector<int> CommentEditing::getNewOrder()
{
    return wind->getNewOrder();
}
