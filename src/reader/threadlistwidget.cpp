#include "threadlistwidget.h"
#include <QDebug>

ThreadListWidget::ThreadListWidget(QWidget *parent, ForumDatabase &f) :
	QTreeWidget(parent), fdb(f) {
    setColumnCount(3);
    currentGroup = 0;
    disableSortAndResize = false;
    QStringList headers;
    headers << "Subject" << "Date" << "Author" << "Ordernum";
    setHeaderLabels(headers);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem *)),
            this, SLOT(messageSelected(QTreeWidgetItem*,QTreeWidgetItem *)));
    hideColumn(3);

    markReadAction = new QAction("Mark thread read", this);
    markReadAction->setToolTip("Marks all messages in this thread read");
    connect(markReadAction, SIGNAL(triggered()), this, SLOT(markReadClicked()));
    markUnreadAction = new QAction("Mark thread unread", this);
    markUnreadAction->setToolTip("Marks all messages in this thread as unread");
    connect(markUnreadAction, SIGNAL(triggered()), this, SLOT(markUnreadClicked()));
    threadPropertiesAction = new QAction("Thread properties", this);
    threadPropertiesAction->setToolTip("Information and settings for selected thread");
    connect(threadPropertiesAction, SIGNAL(triggered()), this, SLOT(threadPropertiesClicked()));
    viewInBrowserAction = new QAction("View in browser", this);
    viewInBrowserAction->setToolTip("View the message in external browser");
    connect(viewInBrowserAction, SIGNAL(triggered()), this, SLOT(viewInBrowserClicked()));
    forceUpdateThreadAction = new QAction("Force update of thread", this);
    forceUpdateThreadAction->setToolTip("Updates all messages in selected thread");
    connect(forceUpdateThreadAction, SIGNAL(triggered()), this, SLOT(forceUpdateThreadClicked()));
}

ThreadListWidget::~ThreadListWidget() {
}

void ThreadListWidget::groupChanged(ForumGroup *grp) {
    if(grp != currentGroup) return;

    if(!grp->isSubscribed()) {
        groupDeleted(grp);
    }
}

void ThreadListWidget::groupDeleted(QObject*g) {
    ForumGroup *grp = static_cast<ForumGroup*>(g);
    if(grp == currentGroup)
        groupSelected(0);
}

void ThreadListWidget::messageFound(ForumMessage *msg) {
    if(msg->thread()->group() == currentGroup) addMessage(msg);
}

void ThreadListWidget::threadFound(ForumThread *thread) {
    if(thread->group() == currentGroup) addThread(thread);
}

void ThreadListWidget::addMessage(ForumMessage *message) {
    Q_ASSERT(message);
    Q_ASSERT(message->thread()->group() == currentGroup);
    // qDebug() << Q_FUNC_INFO << message->toString();
    QPair<QTreeWidgetItem*, ForumThread*> threadPair;

    ThreadListThreadItem *threadItem = forumThreads.key(message->thread());
    Q_ASSERT(threadItem);

    ThreadListMessageItem *item = 0;
    if(message->ordernum() == 0) { // First message - update thread item!
        item = threadItem;
        threadItem->setMessage(message);
        // qDebug() << Q_FUNC_INFO << "setting the thread item";
    } else { // Reply message - create new item
        item = new ThreadListMessageItem(threadItem, message);
    }

    forumMessages[item] = message;
    item->updateItem();
    item->updateRead();

    // Update the thread item
    if(!dynamic_cast<ThreadListThreadItem*> (item)) {
        forumThreads.key(message->thread())->updateUnreads();
    }
    if(!disableSortAndResize) {
        sortItems(3, Qt::AscendingOrder);
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);
    }
}

void ThreadListWidget::removeMessage(ForumMessage *message) {
    ThreadListMessageItem *item = forumMessages.key(message, 0);
    Q_ASSERT(item);
    if(message->ordernum()==0) {
        ThreadListThreadItem *threadItem = qobject_cast<ThreadListThreadItem*> (item);
        Q_ASSERT(threadItem);
        threadItem->setMessage(0);
    } else {
        item->deleteLater();
    }
    disconnect(message, 0, this, 0);
    forumMessages.remove(item);
}

void ThreadListWidget::addThread(ForumThread *thread) {
    Q_ASSERT(thread);
    qDebug() << Q_FUNC_INFO << thread->toString();
    Q_ASSERT(thread->group() == currentGroup);

    connect(thread, SIGNAL(messageAdded(ForumMessage*)), this, SLOT(messageFound(ForumMessage*)));
    connect(thread, SIGNAL(messageRemoved(ForumMessage*)), this, SLOT(removeMessage(ForumMessage*)));

    ThreadListThreadItem *threadItem = new ThreadListThreadItem(this, thread);

    forumThreads[threadItem] = thread;
    addTopLevelItem(threadItem);
    if(!disableSortAndResize) {
        sortItems(3, Qt::AscendingOrder);
        resizeColumnToContents(0);
        resizeColumnToContents(1);
        resizeColumnToContents(2);
    }
}

void ThreadListWidget::removeThread(ForumThread *thread) {
    disconnect(thread, 0, this, 0);
    ThreadListThreadItem *item = forumThreads.key(thread);
    Q_ASSERT(item);
    item->deleteLater();
}

void ThreadListWidget::groupSelected(ForumGroup *fg) {
    qDebug() << Q_FUNC_INFO << fg;
    if(!fg) {
        if(currentGroup) {
            disconnect(currentGroup, 0, this, 0);
            currentGroup = 0;
        }
        clearList();
        emit messageSelected(0);
    }
    if(currentGroup != fg) {
        if(currentGroup) {
            disconnect(currentGroup, 0, this, 0);
        }
        currentGroup = fg;
        connect(currentGroup, SIGNAL(changed(ForumGroup*)), this, SLOT(groupChanged(ForumGroup*)));
        connect(currentGroup, SIGNAL(destroyed(QObject*)), this, SLOT(groupDeleted(QObject*)));
        connect(currentGroup, SIGNAL(threadAdded(ForumThread*)), this, SLOT(threadFound(ForumThread*)));
        connect(currentGroup, SIGNAL(threadRemoved(ForumThread*)), this, SLOT(removeThread(ForumThread*)));
        clearSelection();
        updateList();
        setCurrentItem(topLevelItem(0));
    }
}

void ThreadListWidget::clearList() {
    foreach(ForumMessage *fm, forumMessages.values())
        disconnect(fm, 0, this, 0);
    foreach(ForumThread *ft, forumThreads.values())
        disconnect(ft, 0, this, 0);
    forumMessages.clear();
    forumThreads.clear();
    clear();
}

void ThreadListWidget::updateList() {
    qDebug() << Q_FUNC_INFO;
    if(!currentGroup) return;
    clearList();
    // Add the threads and messages in order
    QList<ForumThread*> threads = currentGroup->values();
    qSort(threads);
    disableSortAndResize = true;
    qDebug() << Q_FUNC_INFO << "group" << currentGroup->toString() << "has" << threads.size() << "threads";
    foreach(ForumThread *thread, threads) {
        qDebug() << Q_FUNC_INFO << "adding" << thread;
        addThread(thread);
        qDebug() << Q_FUNC_INFO << "added" << thread;
        QList<ForumMessage*> messages = thread->values();
        qSort(messages);
        foreach(ForumMessage *message, messages) {
            addMessage(message);
        }
    }
    disableSortAndResize = false;
    sortItems(3, Qt::AscendingOrder);
    resizeColumnToContents(0);
    resizeColumnToContents(1);
    resizeColumnToContents(2);
}

void ThreadListWidget::messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev) {
    Q_UNUSED(prev);
    if (!item)
        return;
    if(dynamic_cast<ThreadListShowMoreItem*> (item)) {
        ThreadListShowMoreItem * smItem = dynamic_cast<ThreadListShowMoreItem*> (item);
        ThreadListThreadItem* tli = dynamic_cast<ThreadListThreadItem*> (smItem->parent());
        setCurrentItem(prev);
        emit moreMessagesRequested(tli->thread());
    } else if (dynamic_cast<ThreadListMessageItem*> (item)) {
        ThreadListMessageItem* msgItem = dynamic_cast<ThreadListMessageItem*> (item);
        ForumMessage *msg = msgItem->message();
        if(!msg) {
            qDebug() << Q_FUNC_INFO << "Thread item with no message? Broken parser?";
        } else {
            Q_ASSERT(msg);
            Q_ASSERT(msg->isSane());
            emit messageSelected(msg);
            msgItem->updateRead();
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Selected an item which s not a showmore or message item. Broken parser?";
        //if(forumThreads.contains(item))
        //    qDebug() << "The thread in question is " << forumThreads.value(item)->toString();
        emit messageSelected(0);
    }
}

void ThreadListWidget::contextMenuEvent(QContextMenuEvent *event) {
    if(itemAt(event->pos())) {
        QMenu menu(this);
        menu.addAction(viewInBrowserAction);
        menu.addAction(markReadAction);
        menu.addAction(markUnreadAction);
        menu.addAction(forceUpdateThreadAction);
        menu.addAction(threadPropertiesAction);
        menu.exec(event->globalPos());
    }
}

void ThreadListWidget::markReadClicked(bool read) {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    ForumMessage *threadMessage = msgItem->message();
    if(threadMessage) {
        foreach(ForumMessage *msg, threadMessage->thread()->values()) {
            msg->setRead(read);
        }
    }
}

void ThreadListWidget::markUnreadClicked() {
    markReadClicked(false);
}

void ThreadListWidget::threadPropertiesClicked() {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    ForumMessage *threadMessage = msgItem->message();
    if(threadMessage) {
        emit threadProperties(threadMessage->thread());
    }
}

void ThreadListWidget::viewInBrowserClicked() {
    emit viewInBrowser();
}

void ThreadListWidget::selectNextUnread() {
    QTreeWidgetItem *newItem = currentItem();
    ThreadListMessageItem *mi = 0;
    while(newItem) {
        if(newItem->childCount()) { // Thread item
            newItem = newItem->child(0);
        } else if(newItem->parent()) { // A message item
            QTreeWidgetItem *nextChild = newItem->parent()->child(newItem->parent()->indexOfChild(newItem) + 1);
            if(!nextChild) { // Jump to next parent
                newItem = topLevelItem(indexOfTopLevelItem(newItem->parent()) + 1);
                if(!newItem) return; // No more threads to search
            } else {
                newItem = nextChild;
            }
        } else { // Single message thread
            newItem = topLevelItem(indexOfTopLevelItem(newItem) + 1);
            if(!newItem) return; // No more threads to search
        }
        ThreadListMessageItem *mi = dynamic_cast<ThreadListMessageItem*> (newItem);
        if(mi && mi->message()) {
            if(!mi->message()->isRead()) {
                setCurrentItem(newItem);
                return;
            }
        }
    }
    /*
    QTreeWidgetItem *newItem = 0;
    ThreadListMessageItem *mi = 0;
    bool isShowMore = false;

    // Find next unread item
    if(item) {
        bool isRead = true;
        do {
            if(item->childCount()) {
                // Is a thread item with child
                newItem = item->child(0);
            } else if(item->parent()) {
                // Is a message item
                QTreeWidgetItem *nextItem = item->parent()->child(item->parent()->indexOfChild(item) + 1);
                if(nextItem) {
                    // ..and has a next item after it
                    newItem = nextItem;
                } else {
                    // ..and is last in thread
                    newItem = topLevelItem(indexOfTopLevelItem(item->parent()) + 1);
                }
            } else {
                // Is a thread item without child
                qDebug() << Q_FUNC_INFO << "Top level item without child";
                newItem = 0;
                if(indexOfTopLevelItem(item) + 1 < topLevelItemCount())
                    newItem = topLevelItem(indexOfTopLevelItem(item) + 1);
            }
            item = newItem;
            if(item) {
                mi = dynamic_cast<ThreadListMessageItem*> (newItem);
                isShowMore = dynamic_cast<ThreadListShowMoreItem*> (newItem);
                isRead = true;
                if(mi && mi->message() && mi->message())
                    isRead = mi->message()->isRead();
                ForumMessage *message = mi->message(); // Shouldn't be 0 but sometimes is
            }
        } while(item && (isShowMore || isRead));
        if(mi) setCurrentItem(mi);
    }
    */
}
void ThreadListWidget::forceUpdateThreadClicked() {
    ThreadListMessageItem *msgItem = dynamic_cast<ThreadListMessageItem*> (currentItem());
    ForumThread *thread = 0;
    if(msgItem) {
        if(msgItem->message() && msgItem->message()->thread()) {
            thread = msgItem->message()->thread();
        } else if(!msgItem->message()) {
            ThreadListThreadItem *threadItem = dynamic_cast<ThreadListThreadItem*> (currentItem());
            if(threadItem) {
                thread = threadItem->thread();
            }
        }
    }
    if(thread)
        emit updateThread(thread, true);
}
