/*
 * threadlistwidget.h
 *
 *  Created on: Nov 7, 2009
 *      Author: vranki
 */

#ifndef THREADLISTWIDGET_H_
#define THREADLISTWIDGET_H_

#include <QTreeWidget>
#include <forumgroup.h>
#include <forumthread.h>
#include <forummessage.h>
#include <forumdatabase.h>

#include "messageformatting.h"

class ThreadListWidget : public QTreeWidget {
	Q_OBJECT

public:
	ThreadListWidget(QWidget *parent, ForumDatabase &f);
	virtual ~ThreadListWidget();
public slots:
	void groupSelected(ForumGroup fg);
	void updateMessageRead(QTreeWidgetItem *item);
	void messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev);
signals:
	void messageSelected(ForumMessage msg);
private:

	QHash<QTreeWidgetItem*, ForumMessage> forumMessages;
	ForumDatabase &fdb;
};

#endif /* THREADLISTWIDGET_H_ */