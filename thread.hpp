#ifndef THREAD_HPP
#define THREAD_HPP

#include <iostream>
#include <QApplication>
#include <QtGui>
#include <QThread>
#include "dbmanager.hpp"

class MusicPlayer;

class Thread : public QThread
{
    Q_OBJECT

    private :
            dbManager*db;
            QString path;
            QMutex *mut;
            QTreeWidget *biblio;
             void insertSong(QStringList song);

    public:
            Thread();


    public slots :
            void run();
            void link(QString path);
            void setParam(QTreeWidget*,QString);

    signals :
            void complete();
};

#endif // MYTHREAD_HPP
