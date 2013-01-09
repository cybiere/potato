#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <phonon>
#include "dbmanager.hpp"
#include "thread.hpp"
#include <QtGui>

/** @class Classe de gestion de la MainWindow */

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QWidget *options;
    QWidget *plBlock;
    QWidget *searchBlock;
    QLineEdit *searchField;
    QTreeWidget *biblio;
    QListWidget *srcDirList;
    QTreeWidget *current;
    QTreeWidget *plists;
    QTreeWidget *searchRes;
    void scanDir(QString);
    void insertSong(QStringList);
    void regenPlaylists();
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionPrev;
    QAction *actionNext;
    QAction *actionLoop;
    QLabel *timeTotal;
    QLabel *timeCurrent;
    QStatusBar *statusBar;
    QString convertTime(qint64);
    Phonon::MediaObject* media;
    Phonon::AudioOutput* output;
    int loopState;
    QTreeWidgetItem *playing;
    void bold();
    void clear();
    Thread* thr;


public:
    MainWindow();
    dbManager *db;

public slots:
    void showOptions();
    void showBiblio();
    void showPlists();
    void showSearch();
    void addSourceDir();
    void delSourceDir();
    void addPl();
    void delPl();
    void refresh();
    void upTimeTot(qint64);
    void incrTimeCur(qint64);
    void addToCurrent(QTreeWidgetItem*,int);
    void addSearchToCurrent(QTreeWidgetItem*,int);
    void play();
    void stop();
    void prev();
    void next();
    void songEnd();
    void loop();
    void selectedSong(QTreeWidgetItem*,int);
    void contextCurrent(QPoint);
    void contextSearch(QPoint);
    void contextBiblio(QPoint);
    void contextSrc(QPoint);
    void contextPl(QPoint);
    void playSelected();
    void addCurrentToPl();
    void addSearchToPl();
    void addBiblioToPl();
    void insertPl(QString,QTreeWidgetItem *,int);
    void changeDockInfo(QTreeWidgetItem *,int);
    void changeSearch(QString);
    void saveCurrent();
    void loadCurrent();
    void regenBiblio();
    void clearCurrent();
    void currentToPl();
    void changeStatus(QString);

signals :
    void changeWikiInfo(QString);

};

#endif // MAINWINDOW_HPP
