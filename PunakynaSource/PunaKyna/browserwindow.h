#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QWidget>

class Settings;
#include "browsermodel.h"

class QString;
class QPoint;
class QTreeWidgetItem;
class QTreeWidget;
class QLineEdit;
template<typename T> class QVector;

class Settings;

class BrowserWindow : public QWidget
{
    Q_OBJECT

public:
    BrowserWindow(Settings *set, QWidget *parent = 0);
    virtual ~BrowserWindow();

    void setBrowserModel(BrowserModel *model);

    Indexer currentIndex();

    bool selectNext();
    bool selectPrev();

private slots:
    void on_filterTypeChange(int ind);
    void on_filterChange(const QString &text);

    void on_sortChange(int ind);

    void on_dataChange(Indexer of, BrowserModel::DataType slot);

    void on_currentChanged(QTreeWidgetItem *current, QTreeWidgetItem *);

private:
    void contextMenuEvent(QContextMenuEvent *e);

    enum viewSlots
    {
        Slot_Name, Slot_State, Slot_Ident, Slot_Index, Slot_Null
    };

    QWidget *createFilterWidget();
    bool showAll(QTreeWidgetItem *item, const QString &cond = QString());

    QWidget *createSortWidget();

    Indexer createIndex(QTreeWidgetItem *of);

    QTreeWidgetItem *populateBrowser(Indexer ind);
    void setItemDisplay(QTreeWidgetItem *of, int toState);

    bool changeState(QTreeWidgetItem *of, int to);

signals:
    void entrySelected(Indexer&);
    void badEntry(Indexer&);

    void browsingComplete();

private:
    Settings *_settings;

    BrowserModel *_model;
    QTreeWidget *_view;

    int _filterType;
    QLineEdit * _filterField;

    Indexer _tempIndex;
};

#endif // BROWSERWINDOW_H
