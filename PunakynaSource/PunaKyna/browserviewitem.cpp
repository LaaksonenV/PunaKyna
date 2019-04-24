#include "browserviewitem.h"

BrowserViewItem::BrowserViewItem(const QStringList &strings)
    : QTreeWidgetItem(strings)
{
}

BrowserViewItem::BrowserViewItem(QTreeWidget *parent, const QStringList &strings)
    : QTreeWidgetItem(parent, strings)
{
}

BrowserViewItem::BrowserViewItem(QTreeWidget *parent)
    : QTreeWidgetItem(parent)
{
}

BrowserViewItem::BrowserViewItem(BrowserViewItem *parent, const QStringList &strings)
    : QTreeWidgetItem(parent, strings)
{
}

BrowserViewItem::BrowserViewItem(BrowserViewItem *parent)
    : QTreeWidgetItem(parent)
{
}
