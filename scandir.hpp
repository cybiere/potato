#ifndef SCANDIR_HPP
#define SCANDIR_HPP

#include <iostream>
#include <QApplication>
#include <QtGui>
#include <QThread>
#include "dbmanager.hpp"
#include "time.h"

/** @class Classe de gestion du thread pour le parcours des répertoires et l'ajout dans la bdd. */

class ScanDir : public QThread
{
    Q_OBJECT

    private :
            dbManager*db;
            QMutex* mut;
            QStringList waitList;
            struct timespec req;

    public:
            ScanDir(QMutex*);


    public slots :
            void run();
            void link();
            void setParam(QStringList);

    signals :
            void refresh();
            void finish(QString);
};

#endif // SCANDIR_HPP
