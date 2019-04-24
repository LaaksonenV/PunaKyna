#include "browserwindow.h"

#include <QTreeWidget>

#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QDesktopServices>
#include <QContextMenuEvent>

#include "smalldialogs.h"
#include "settings.h"

/*!
 * \class BrowserWindow
 * \brief The BrowserWindow class is the view of the folderbrowser M/V
 * architecture
 *
 * Current implementation simply contains a QTreeWidget and acts like a
 * controlling composite widget.
 *
 * The view receives changes to the model by the on_dataChange() slot
 */

/*!
 * \brief BrowserWindow::BrowserWindow Construct the view widget
 * \param set Setting file
 * \param parent
 */
BrowserWindow::BrowserWindow(Settings *set, QWidget *parent)
    : QWidget(parent)
    , _settings(set)
    , _model(nullptr)
    , _filterType(0)
    , _filterField(nullptr)
    , _tempIndex(Indexer())
{
    QVBoxLayout *lay = new QVBoxLayout(this);

    lay->addWidget(createFilterWidget());
//    lay->addWidget(createSortWidget());

    _view = new QTreeWidget(this);
    lay->addWidget(_view,2);

    _view->setSelectionMode(QAbstractItemView::SingleSelection);
    _view->setHeaderHidden(true);
    connect(_view, &QTreeWidget::currentItemChanged,
            this, &BrowserWindow::on_currentChanged);
    _view->setFocusPolicy(Qt::NoFocus);
}

/*!
 * \brief BrowserWindow::~BrowserWindow Basic destructor
 */
BrowserWindow::~BrowserWindow()
{
    _model = nullptr;
}

/*!
 * \brief BrowserWindow::createFilterWidget Creates a composite widget for
 * filtering the view.
 * \return Composite QWidget
 * \internal
 *
 * Add new filtertypes also to slot on_filterTypeChange(), and edit
 * slot on_filterChange() and function showAll() accordingly
 */
QWidget *BrowserWindow::createFilterWidget()
{
    QWidget *ret = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(ret);

    lay->addWidget(new QLabel(tr("Suodata:")));

    QComboBox *filterType = new QComboBox();
    filterType->addItem("");
    filterType->addItem(tr("Katsomattomat"));
    QFont ffont;
    ffont.setBold(true);
    filterType->setItemData(filterType->count()-1, ffont, Qt::FontRole);
    filterType->addItem(tr("Arvostelemattomat"));
    filterType->addItem(QIcon(":/icons/confl"), tr("Tarkistettavat"));
    filterType->addItem(QIcon(":/icons/ch"), tr("Arvostellut"));
    filterType->addItem(QIcon(":/icons/lck"), tr("Lukitut"));
    filterType->addItem(QIcon(":/icons/arch"), tr("Valmiit"));
    filterType->addItem(tr("Opiskelijan numerolla"));
    filterType->addItem(tr("Kysymyksen numerolla"));


    connect(filterType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(on_filterTypeChange(int)));

    lay->addWidget(filterType);

    _filterField = new QLineEdit();
    _filterField->setEnabled(false);
    connect(_filterField, &QLineEdit::textChanged,
            this, &BrowserWindow::on_filterChange);

    lay->addWidget(_filterField);

    return ret;
}

/*!
 * \brief BrowserWindow::on_filterTypeChange Slot for filtering
 * \param ind index of QComboBox containing filter types
 * \internal
 */
void BrowserWindow::on_filterTypeChange(int ind)
{
    if (!_view->topLevelItemCount())
        return;

    _filterType = ind;

    // unless filtering by text, filtering is activated when selecting condition
    if (ind == 0)
        showAll(_view->topLevelItem(0));
    else if (ind == 1)
        showAll(_view->topLevelItem(0), "unviewed");
    else if (ind == 2)
        showAll(_view->topLevelItem(0), "uncommented");
    else if (ind == 3)
        showAll(_view->topLevelItem(0), "conflicting");
    else if (ind == 4)
        showAll(_view->topLevelItem(0), "commented");
    else if (ind == 5)
        showAll(_view->topLevelItem(0), "locked");
    else if (ind == 6)
        showAll(_view->topLevelItem(0), "completed");

/*    if (ind > 7)
        _filterField->setHidden(false);
    else
    {
        _filterField->clear();
        _filterField->setHidden(true);
    }*/
}

/*!
 * \brief BrowserWindow::on_filterChange Filter by text
 * \param text Expression to filter by
 * \internal
 */
void BrowserWindow::on_filterChange(const QString &text)
{
    if (!_view->topLevelItemCount())
        return;

    QString cond;

    if (_filterType == 3)
        cond = "StudId,";
    else if (_filterType == 5)
        cond = "QuestId";
    cond += text;
    showAll(_view->topLevelItem(0), cond);
}

/*!
 * \brief BrowserWindow::showAll Filter view
 * \param item QTreeWidgetItem to check
 * \param cond Condition for filtering
 * \return \c true if condition matches and item is shown, false otherwise
 * \internal
 *
 * Checks if the item matches the given condition, and recursively doeas the
 * same for all items under it, then hiding the item if it would show empty.
 */
bool BrowserWindow::showAll(QTreeWidgetItem *item, const QString &cond)
{
    if (item->text(Slot_State).toInt() == GradingState::Empty ||
        item->text(Slot_State).toInt() == GradingState::Wrong)
    {
        item->setHidden(true);
        return false;
    }

    bool condMatch = false;
    bool done = false;

    if (cond.isEmpty())
        condMatch = true;
    else
    {
        QString first = cond.section(',',0,0);
        if (first == "commented")
        {
            if (item->text(Slot_State).toInt() == GradingState::Commented)
                condMatch = true;
        }
        else if (first == "conflicting")
        {
            if (item->text(Slot_State).toInt() == GradingState::Conflicting)
                condMatch = true;
        }
        else if (first == "unviewed")
        {
            if (item->text(Slot_State).toInt() == GradingState::UnTouched)
                condMatch = true;
        }
        else if (first == "uncommented")
        {
            if (item->text(Slot_State).toInt() < GradingState::Commented)
                condMatch = true;
        }
        else if (first == "locked")
        {
            if (item->text(Slot_State).toInt() == GradingState::Locked)
                condMatch = true;
        }
        else if (first == "completed")
        {
            if (item->text(Slot_State).toInt() == GradingState::Sent)
                condMatch = true;
        }
        else if (first == "StudId")
        {
            if (item->childCount())
            {
                if (item->text(Slot_Ident).contains(cond.section(',',1)))
                {
                    condMatch = true;
                }
                done = true;
            }
        }
        else if (first == "QuestId")
        {
            if (item->parent() && !item->parent()->parent())
            {
                if (item->text(Slot_Ident).contains(cond.section(',',1)))
                {
                    condMatch = true;
                }
                done = true;
            }
        }
    }

    if (condMatch)
        item->setHidden(false);
    else
        item->setHidden(true);

    if (!done && item->childCount())
    {
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (showAll(item->child(i), cond))
                done = true;
        }
        if (!done)
            item->setHidden(true);
        else
            item->setHidden(false);
    }
    return !(item->isHidden());
}

/*!
 * \brief BrowserWindow::createSortWidget Creates a composite QWidget for
 * sorting view
 * \return Composite QWidget
 * \internal
 *
 * Currently unused, and waiting for properly implemented view
 */
QWidget *BrowserWindow::createSortWidget()
{
    QWidget *ret = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(ret);

    lay->addWidget(new QLabel(tr("Järjestä:")));

    QComboBox *sortType = new QComboBox();
    sortType->addItem(tr("Kysymyksen mukaan"));
    sortType->addItem(tr("Arvostelutilanteen mukaan"));
    sortType->addItem(tr("Opiskelijan mukaan"));

    connect(sortType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(on_sortChange(int)));

    lay->addWidget(sortType);

    return ret;
}

void BrowserWindow::on_sortChange(int ind)
{
    if (ind == 0)
        _view->setSortingEnabled(false);
 /*   else
    {
        if (ind == 1)
            _view->sortItems(BrowserModel::State, Qt::AscendingOrder);
        if (ind == 2)
            _view->sortItems(StudId, Qt::AscendingOrder);
        _view->setSortingEnabled(true);
    }*/
}

/*!
 * \brief BrowserWindow::setBrowserModel Sets the model for view
 * \param model BrowserModel
 *
 * Clears the view, and sets a new model. Old model is not destroyed
 */
void BrowserWindow::setBrowserModel(BrowserModel *model)
{
    if (_model)
    {
        disconnect(_model, &BrowserModel::dataChanged,
                   this, &BrowserWindow::on_dataChange);
    }
    _model = model;
    Indexer ind;
    _view->clear();
    QTreeWidgetItem *topitem = populateBrowser(ind);
    _view->addTopLevelItem(topitem);
    for (int c = 0; c < _view->columnCount(); ++c)
        _view->hideColumn(c);
    _view->showColumn(Slot_Name);
    showAll(topitem);
    if(!connect(_model, &BrowserModel::dataChanged,
            this, &BrowserWindow::on_dataChange))
        return;
    _view->setContextMenuPolicy(Qt::CustomContextMenu);

}

/*!
 * \brief BrowserWindow::currentIndex Returns the currently selected items
 * index
 * \return Indexer of current item, or empty Indexer if no item is selected
 */
Indexer BrowserWindow::currentIndex()
{
    if (_view->currentItem())
        return createIndex(_view->currentItem());
    _tempIndex.clear();
    return _tempIndex;
}

/*!
 * \brief BrowserWindow::selectNext Selects the next item that is visually
 * below current item
 * \return \c false if current item is last, \c true otherwise
 *
 * The next selected item is on the lowest level of the tree. Thus if
 * function is called when current item is at top or middle of the tree, the
 * new selection will be the first branch with no subbranches
 *
 * If the curent item is last in line, the next item is the top item.
 *
 * \sa selectPrev()
 */
bool BrowserWindow::selectNext()
{
    if (!_model)
        return false;

    clearFocus();

    // get the current item, or top item
    QTreeWidgetItem *current = _view->currentItem();
    if (!current)
        current = _view->topLevelItem(0);

    // get the current items index, and parent
    QTreeWidgetItem *upper = current->parent();
    int currentIndex = 0;
    if (upper)
        currentIndex = upper->indexOfChild(current);

    // if current is not at bottom of the tree, now it is, unless the bottom
    // node is hidden
    if (current->childCount() > 0)
    {
        while (current->childCount() > 0)
        {
            current = current->child(0);
        }
        if (!current->isHidden())
        {
            _view->setCurrentItem(current);
            return true;
        }
        upper = current->parent();
        currentIndex = 0;
    }

    // Traverse tree until next eligible has been found, or inform browsing
    // complete if tree has been traversed without eligibles
    while (true)
    {
        // while last branch, go up
        while (upper && currentIndex >= upper->childCount() -1)
        {
            current = upper;
            upper = upper->parent();
            currentIndex = 0;
            if (upper)
                currentIndex = upper->indexOfChild(current);
        }
        // if last branch until at top (no parent, index = 0)
        if (!upper)
        {
            _view->setCurrentItem(current);
            emit browsingComplete();
            return false;
        }

        // pick the next branch, and go to its first bottoms if needed
        ++currentIndex;
        current = upper->child(currentIndex);
        while (current->childCount())
        {
            upper = current;
            current = current->child(0);
            currentIndex = 0;
        }
        if (!current->isHidden())
        {
            _view->setCurrentItem(current);
            return true;
        }
        // rinse & repeat, until next is found, or at top
    }

}

/*!
 * \brief BrowserWindow::selectPrev Selects the previous item that is visually
 * above current item
 * \return \c false if current item is first, \c true otherwise
 *
 * The previous selected item is on the lowest level of the tree. Thus if
 * function is called when current item is at top or middle of the tree, the
 * new selection will be the first branch with no subbranches
 *
 * If the curent item is first in line, the next item is the top item.
 *
 * \sa selectNext()
 */
bool BrowserWindow::selectPrev()
{
    // get the current item, or top item
    QTreeWidgetItem *current = _view->currentItem();
    if (!current)
        current = _view->topLevelItem(0);

    // get the current items index, and parent
    QTreeWidgetItem *upper = current->parent();
    int currentIndex = 0;
    if (upper)
        currentIndex = upper->indexOfChild(current);

    // Traverse tree until next eligible has been found, or inform browsing
    // complete if tree has been traversed without eligibles
    while (true)
    {
        // while first branch, go up
        while (upper && currentIndex == 0)
        {
            current = upper;
            upper = upper->parent();
            currentIndex = 0;
            if (upper)
                currentIndex = upper->indexOfChild(current);
        }
        // if first branch untli at top (no parent, index = 0)
        if (!upper)
        {
            _view->setCurrentItem(current);
            emit browsingComplete();
            return false;
        }

        // pick the previous branch, and go to its last sub brach
        --currentIndex;
        current = upper->child(currentIndex);
        while (current->childCount())
        {
            upper = current;
            currentIndex = current->childCount()-1;
            current = current->child(current->childCount()-1);
        }
        if (!current->isHidden())
        {
            _view->setCurrentItem(current);
            return true;
        }
        // rinse & repeat, until previous is found, or at top
    }

}

/*!
 * \brief BrowserWindow::on_dataChange Slot that model takes models signals for
 * data change
 * \param of Indexer of changed item
 * \param slot DataType of changed data
 * \internal
 *
 * After receiving signal, the type of item that changed and the slot of
 * changed data is checked, and relevant data fetched from the model
 */
void BrowserWindow::on_dataChange(Indexer of,
                                  BrowserModel::DataType slot)
{
    if (!_view->topLevelItemCount())
        return;

    // Look for item corresponding to indexer. If treewidgets ordering is out of
    // whack, more work is needed than just selecting children by indexers
    QTreeWidgetItem *itm = _view->topLevelItem(0);
    if (of.quest >= 0)
    {
        itm = itm->child(of.quest);
        if (itm->text(Slot_Index).toInt() != of.quest)
        {
            itm = itm->parent();
            for (int i = 0; i < itm->childCount(); ++i)
                if (itm->child(i)->text(Slot_Index).toInt() == of.quest)
                {
                    itm = itm->child(i);
                    break;
                }
        }
    }
    if (of.answ >= 0)
    {
        itm = itm->child(of.answ);
        if (itm->text(Slot_Index).toInt() != of.answ)
        {
            itm = itm->parent();
            for (int i = 0; i < itm->childCount(); ++i)
                if (itm->child(i)->text(Slot_Index).toInt() == of.answ)
                {
                    itm = itm->child(i);
                    break;
                }
        }
    }

    if (slot == BrowserModel::State)
    {
        int to = _model->getText(of, slot).toInt();
        itm->setText(Slot_State, QString::number(to));

        setItemDisplay(itm, to);
    }
    else if (slot == BrowserModel::DisplayName)
        itm->setText(Slot_Name, _model->getText(of, slot));
}

void BrowserWindow::on_currentChanged(QTreeWidgetItem *current,
                                      QTreeWidgetItem *)
{
    if (!current)
        return;

    Indexer ind = createIndex(current);
    emit entrySelected(ind);
}

void BrowserWindow::contextMenuEvent(QContextMenuEvent *e)
{
    QTreeWidgetItem *it = _view->itemAt(e->pos());
    if (!it)
        return;

    QString url = _model->getText(createIndex(it), BrowserModel::Url);
    QAction *link = new QAction(tr("Avaa Exam sivu"), this);
    link->setStatusTip(tr("Avaa valitun tentin tai tenttijän Exam sivu oletusselaimessa."));
    connect(link, &QAction::triggered,
            [&]{QDesktopServices::openUrl(QUrl(url));});

    QMenu menu(this);
    menu.addAction(link);
    menu.exec(e->globalPos());
    e->accept();
}


Indexer BrowserWindow::createIndex(QTreeWidgetItem *of)
{
    _tempIndex.clear();
    if (of->parent())
    {
        _tempIndex.quest = of->text(Slot_Index).toInt();
        if (of->parent()->parent())
        {
            _tempIndex.answ = _tempIndex.quest;
            _tempIndex.quest = of->parent()->text(Slot_Index).toInt();
            if (of->parent()->parent()->parent())
            {
                _tempIndex.xtr = _tempIndex.answ;
                _tempIndex.answ = _tempIndex.quest;
                _tempIndex.quest = of->parent()->parent()->text(Slot_Index).toInt();
            }
        }
    }
    return _tempIndex;
}

QTreeWidgetItem *BrowserWindow::populateBrowser(Indexer ind)
{
    QStringList labels;
    for (int i = 0; i < Slot_Null; ++i)
        labels << "";
    labels[Slot_State] = _model->getText(ind, BrowserModel::State);
    int state = labels.at(Slot_State).toInt();
    if (state == GradingState::Wrong ||
        state == GradingState::Empty)
    {
        emit badEntry(ind);
    }
    labels[Slot_Name] = _model->getText(ind, BrowserModel::DisplayName);
    labels[Slot_Ident] = _model->getText(ind, BrowserModel::Identifier);
    if (ind.quest >= 0)
    {
        if (ind.answ >= 0)
        {
            if (ind.xtr >= 0)
                labels[Slot_Index] = QString::number(ind.xtr);
            else
                labels[Slot_Index] = QString::number(ind.answ);
        }
        else
            labels[Slot_Index] = QString::number(ind.quest);
    }
    else
        labels[Slot_Index] = QString::number(-1);

    QTreeWidgetItem *ret = new QTreeWidgetItem(labels);

    setItemDisplay(ret, labels.at(Slot_State).toInt());

    QTreeWidgetItem *newchild;

    for (int i = 0; i < _model->indexCount(ind); ++i)
    {
        if (ind.quest < 0)
        {
            ind.quest = i;
            newchild = populateBrowser(ind);
            ind.quest = -1;
        }
        else if (ind.answ < 0)
        {
            ind.answ = i;
            newchild = populateBrowser(ind);
            ind.answ = -1;
        }
        else
        {
            ind.xtr = i;
            newchild = populateBrowser(ind);
            ind.xtr = -1;
        }
        ret->addChild(newchild);
    }
    return ret;
}

void BrowserWindow::setItemDisplay(QTreeWidgetItem *of, int toState)
{
    QFont fontt;
    if (toState == GradingState::UnTouched)
    {
        fontt.setBold(true);
        of->setIcon(Slot_Name, QIcon());
    }

    of->setFont(Slot_Name, fontt);

    if (toState == GradingState::Locked)
        of->setIcon(Slot_Name, QIcon(":/icons/lck"));
    else if (toState == GradingState::Conflicting)
        of->setIcon(Slot_Name, QIcon(":/icons/confl"));
    else if (toState == GradingState::Commented)
        of->setIcon(Slot_Name, QIcon(":/icons/ch"));
    else if (toState == GradingState::AutoCommented)
        of->setIcon(Slot_Name, QIcon(":/icons/auch"));
    else if (toState == GradingState::Sent)
        of->setIcon(Slot_Name, QIcon(":/icons/arch"));
//    else if (toState == GradingState::Empty ||
  //           toState == GradingState::Wrong)
    //{
    //}
    else
        of->setIcon(Slot_Name, QIcon());
}
