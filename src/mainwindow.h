#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QList>
#include <QDesktopServices>

#include <parserdatabase.h>
#include <forumdatabase.h>
#include <forumsubscription.h>
#include <forumgroup.h>
#include <forumthread.h>
#include <forummessage.h>

#include "favicon.h"

#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ParserDatabase &pd, ForumDatabase &fd, QWidget *parent = 0 );
    ~MainWindow();
    void updateForumList();
    int getSelectedForum();
signals:
	void subscribeForum();
	void unsubscribeForum(int forumid);
	void updateClicked();
	void updateClicked(int forumid);
	void cancelClicked();
	void groupSubscriptions(int forum);
	void messageRead(ForumMessage message);
public slots:
	void subscribeForumSlot();
	void unsubscribeForumSlot();
	void groupSubscriptionsSlot();
	void updateClickedSlot();
	void updateSelectedClickedSlot();
	void cancelClickedSlot();
	void viewInBrowserClickedSlot();
	void setForumStatus(int forum, bool reloading);
	void groupSelected(QListWidgetItem* item, QListWidgetItem *prev);
	void messageSelected(QTreeWidgetItem* item, QTreeWidgetItem *prev);
	void iconUpdated(int forum, QIcon newIcon);
private:
	void updateMessageRead(QTreeWidgetItem *item);
    Ui::MainWindowClass ui;
    ParserDatabase &pdb;
    ForumDatabase &fdb;
    ForumMessage displayedMessage;
    QHash<int, int> forumItems;
    QHash<int, Favicon*> forumIcons;
    QHash<QListWidgetItem*, ForumGroup> forumGroups;
    QHash<QTreeWidgetItem*, ForumMessage> forumMessages;
    int busyForums;
};

#endif // MAINWINDOW_H
