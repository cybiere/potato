#ifndef DBMANAGER_HPP
#define DBMANAGER_HPP
#include <QtCore>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <QtSql>
#include <QImage>

/** @class Classe de gestion de la BDD */

class dbManager : public QObject
{
    Q_OBJECT

    QSqlDatabase db;
    QSqlQuery query;
    static dbManager* singleton;
    dbManager();


public:

    static dbManager* getInstance();
    QStringList getSrcDirs();
    void addSrc(QString);
    void delSrc(QString);
    QList<QStringList> *getBiblio();
    QStringList *getAssos(QString);
    QStringList *getPlaylists();
    QString getTitleFromPath(QString path);
    QStringList getSong(QString);
    QStringList getSong(QString,QString,QString);
    bool addPl(QString);
    void delPl(QString);
    int incrNb_played(QString,int);
    void addSgToPl(QString,QString);
    bool getCover(QString path, QImage *Image);


public slots :
    QStringList addSong(QString);
};

#endif // DBMANAGER_HPP
