#ifndef THREADLISTTHREADITEM_H
#define THREADLISTTHREADITEM_H

#include "threadlistshowmoreitem.h"
#include "threadlistmessageitem.h"
#include <QObject>

class ForumMessage;
class ForumThread;

class ThreadListThreadItem : public ThreadListMessageItem
{
    Q_OBJECT

public:
    ThreadListThreadItem(QTreeWidget *tree, ForumThread *_thread);
    void setMessage(ForumMessage *message);
    ForumThread* thread();
public slots:
    void unreadCountChanged();
    void unreadCountChanged(ForumThread *thr);
    virtual void updateItem();
    void threadDeleted();
    void threadMessageDeleted(); // Called when the message is deleted
    void addMessage(ForumMessage *msg);
    void removeMessage(ForumMessage *msg);
private:
    ForumThread *_thread;
    ThreadListShowMoreItem* showMoreItem;
    QTreeWidget *treeWidget;
};

#endif // THREADLISTTHREADITEM_H
