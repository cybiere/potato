#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP
#include <QtCore>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <QtSql>


class dbManager
{
    QSqlDatabase db;
    QSqlQuery query;
public:
    dbManager();
    void addSong(QString file);
    QStringList getSrcDirs();
};

#endif // DBMANAGER_HPP
