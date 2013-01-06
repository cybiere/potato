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
    Phonon::MediaObject* media;
    Phonon::AudioOutput* output;

public:
    MainWindow();
    dbManager *db;

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
