#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP
#include <QtCore>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <QtSql>


class dbManager : public QObject
{
    Q_OBJECT

    QSqlDatabase db;
    QSqlQuery query;
public:
    dbManager();
    QStringList getSrcDirs();
    void addSrc(QString);
    void delSrc(QString);
    QList<QStringList> *getBiblio();
    QStringList getSong(QString,QString,QString);

public slots :
    QStringList addSong(QString);
};

#endif // DBMANAGER_HPP
