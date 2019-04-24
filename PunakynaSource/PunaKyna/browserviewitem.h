#ifndef BROWSERVIEWITEM_H
#define BROWSERVIEWITEM_H

#include <QTreeWidgetItem>

class BrowserViewItem : public QTreeWidgetItem
{
public:
    BrowserViewItem(const QStringList &strings);
    BrowserViewItem(QTreeWidget *parent, const QStringList &strings);
    BrowserViewItem(QTreeWidget *parent);
    BrowserViewItem(BrowserViewItem *parent, const QStringList &strings);
    BrowserViewItem(BrowserViewItem *parent);
};

#endif // BROWSERVIEWITEM_H
