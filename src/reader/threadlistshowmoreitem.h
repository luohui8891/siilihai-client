#ifndef THREADLISTSHOWMOREITEM_H
#define THREADLISTSHOWMOREITEM_H

#include <QTreeWidgetItem>
#include <siilihai/forumdata/forumthread.h>

class ThreadListShowMoreItem : public QTreeWidgetItem
{
public:
    ThreadListShowMoreItem(QTreeWidgetItem *threadItem);
};

#endif // THREADLISTSHOWMOREITEM_H
