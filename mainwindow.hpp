#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <phonon>
#include "dbmanager.hpp"
#include <QtGui>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QWidget *options;
    QWidget *plBlock;
    QTreeWidget *biblio;
    QListWidget *srcDirList;
    QTreeWidget *current;
    QTreeWidget *plists;
    void scanDir(QString);
    void insertSong(QStringList);
    void regenBiblio();
    void regenPlaylists();
    QAction *actionPlay;
    QAction *actionStop;
    QAction *actionPrev;
    QAction *actionNext;
    QAction *actionLoop;
    QLabel *timeTotal;
    QLabel *timeCurrent;
    QString convertTime(qint64);
    Phonon::MediaObject* media;
    Phonon::AudioOutput* output;
    int loopState;
    QTreeWidgetItem *playing;
    void bold();
    void clear();

public:
    MainWindow();
    dbManager *db;

public slots:
    void showOptions();
    void showBiblio();
    void showPlists();
    void addSourceDir();
    void delSourceDir();
    void addPl();
    void delPl();
    void refresh();
    void upTimeTot(qint64);
    void incrTimeCur(qint64);
    void addToCurrent(QTreeWidgetItem*,int);
    void play();
    void stop();
    void prev();
    void next();
    void songEnd();
    void loop();
    void selectedSong(QTreeWidgetItem*,int);
    void contextCurrent(QPoint);
    void contextBiblio(QPoint);
    void contextSrc(QPoint);
    void contextPl(QPoint);
    void playSelected();
    void addCurrentToPl();
    void addBiblioToPl();
    void insertPl(QString,QTreeWidgetItem *,int);

};

#endif // MAINWINDOW_HPP
