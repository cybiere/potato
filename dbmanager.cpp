#include "dbmanager.hpp"
#include <iostream>

dbManager::dbManager()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("PotatoBase");

    query = QSqlQuery(db);

    if(!db.open())
    {
        std::cerr << "Unable to create/open database" << std::endl;
        exit(1);
    }

    //Création des tables si nécessaire
    query = db.exec("CREATE TABLE IF NOT EXISTS song(file string primary key, artist string, title string, album string, genre string, nb_played integer, rating interger)");
    query = db.exec("CREATE TABLE IF NOT EXISTS playlist(id integer primary key autoincrement, name string)");
    query = db.exec("CREATE TABLE IF NOT EXISTS asso_song_pl(pl integer, sg string)");
    query = db.exec("CREATE TABLE IF NOT EXISTS srcDir(dir string)");
}

QStringList dbManager::addSong(QString file)
{
    QStringList song;
    query.prepare("SELECT artist,title,album,genre,nb_played,rating FROM song WHERE file=?");
        query.bindValue(1,file);
    query.exec();
    if(!query.isValid())
    {
        TagLib::FileRef f(file.toStdString().c_str());
        TagLib::Tag *tag = f.tag();

        query.prepare("INSERT INTO song (file, artist, title, album, genre, nb_played, rating) VALUES (?, ?, ?, ?, ?, 0, 0)");
             query.bindValue("1", file);
             query.bindValue("2", QString::fromUtf8(tag->artist().toCString(true)));
             query.bindValue("3", QString::fromUtf8(tag->title().toCString(true)));
             query.bindValue("4", QString::fromUtf8(tag->album().toCString(true)));
             query.bindValue("5", QString::fromUtf8(tag->genre().toCString(true)));
        query.exec();
        query.prepare("SELECT artist,title,album,genre,nb_played,rating FROM song WHERE file=?");
            query.bindValue("1",file);
        query.exec();
    }

    query.first();

    song << query.value(1).toString() << query.value(0).toString() << query.value(2).toString();
    return song;
}


QStringList dbManager::getSrcDirs()
{
    QStringList sources;
    query.prepare("SELECT dir FROM srcDir");
    query.exec();
    while (query.next()) {
        sources << query.value(0).toString();
    }
    return sources;
}

void dbManager::addSrc(QString dir)
{
    query.prepare("INSERT INTO srcDir (dir) VALUES (?)");
         query.bindValue("1", dir);
    query.exec();
}

void dbManager::delSrc(QString dir)
{
    query.prepare("DELETE FROM srcDir WHERE dir=?");
         query.bindValue("1", dir);
    query.exec();
}


QList<QStringList> *dbManager::getBiblio()
{
    QList<QStringList> *songs = new QList<QStringList>;
    query.prepare("SELECT artist,title,album FROM song");
    query.exec();
    while(query.next())
    {
        QStringList song(query.value(1).toString());
        song << query.value(0).toString() << query.value(2).toString();
        songs->append(song);
    }

    return songs;
}
