#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include "dbmanager.hpp"
#include <QtGui>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QWidget *options;
    QTreeWidget *biblio;
    QListWidget *srcDirList;
    void scanDir(QString);
    void insertSong(QStringList);
    void regenBiblio();

public:
    MainWindow();
    dbManager *db;

public slots:
    void showOptions();
    void showBiblio();
    void addSourceDir();
    void delSourceDir();
    void refresh();
};

#endif // MAINWINDOW_HPP
