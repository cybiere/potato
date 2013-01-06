#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include "dbmanager.hpp"
#include "mediaplayer.hpp"
#include <QtGui>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QWidget *options;
    QTreeWidget *biblio;
    QListWidget *srcDirList;
    QTreeWidget *current;
    void scanDir(QString);
    void insertSong(QStringList);
    void regenBiblio();
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionPrev;
    QAction *actionNext;
    QAction *actionLoop;
    QLabel *timeTotal;
    QLabel *timeCurrent;
    QString convertTime(qint64);

public:
    MainWindow();
    dbManager *db;
    MediaPlayer *player;

public slots:
    void showOptions();
    void showBiblio();
    void addSourceDir();
    void delSourceDir();
    void refresh();
    void upTimeTot(qint64);
    void incrTimeCur(qint64);
    void addToCurrent(QTreeWidgetItem*,int);
};

#endif // MAINWINDOW_HPP
