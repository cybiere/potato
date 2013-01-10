#ifndef INSERTPL_H
#define INSERTPLL_H

#include <iostream>
#include <QApplication>
#include <QtGui>
#include <QThread>
#include "dbmanager.hpp"
#include "time.h"

/** @class Classe de gestion du thread pour le parcours des répertoires et l'ajout dans la bdd. */

class InsertPl : public QThread
{
    Q_OBJECT

    private :
            dbManager*db;
            QString playlist;
            QTreeWidgetItem * item;
            QMutex * mut;
            QTreeWidget* plists;
            QTreeWidget* searchRes;
            QTreeWidget* current;
            QTreeWidgetItem* waitlist;
            int niveau;
            struct timespec req;

    public:
            InsertPl(QMutex*,QTreeWidget*,QTreeWidget*,QTreeWidget*);


    public slots :
            void run();
            void insert(QString,QTreeWidgetItem*,int);
            void setParam(QString,QTreeWidgetItem*,int,QTreeWidgetItem* wait = NULL);

    signals :
            void changestatusBar(QString);
};


#endif // INSERTPL_H
