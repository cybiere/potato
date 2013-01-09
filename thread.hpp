#ifndef THREAD_HPP
#define THREAD_HPP

#include <iostream>
#include <QApplication>
#include <QtGui>
#include <QThread>
#include "dbmanager.hpp"
#include "time.h"

class MusicPlayer;

/** @class Classe de gestion du thread pour le parcours des répertoires et l'ajout dans la bdd. */

class Thread : public QThread
{
    Q_OBJECT

    private :
            dbManager*db;
            QStringList waitList;
            struct timespec req;

    public:
            Thread();


    public slots :
            void run();
            void link();
            void setParam(QStringList);

    signals :
            void refresh();
            void finish(QString);
};

#endif // MYTHREAD_HPP
