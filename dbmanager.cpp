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

void dbManager::addSong(QString file)
{
    query.prepare("SELECT file FROM song WHERE file=?");
        query.bindValue(1,file);
    query.exec();
    if(!query.isValid())
    {
        TagLib::FileRef f(file.toStdString().c_str());
        TagLib::Tag *tag = f.tag();

        query.prepare("INSERT INTO song (file, artist, title, album, genre) VALUES (?, ?, ?, ?, ?, 0, 0)");
             query.bindValue("1", file);
             query.bindValue("2", QString::fromUtf8(tag->artist().toCString(true)));
             query.bindValue("3", QString::fromUtf8(tag->title().toCString(true)));
             query.bindValue("4", QString::fromUtf8(tag->album().toCString(true)));
             query.bindValue("5", QString::fromUtf8(tag->genre().toCString(true)));
        query.exec();
    }
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
