#ifndef TAGEDITOR_HPP
#define TAGEDITOR_HPP
#include <QtGui>
#include "dbmanager.hpp"

class TagEditor : public QDialog
{
    Q_OBJECT

    dbManager *db;
    QString path;
    QLineEdit *genre;
    QLineEdit *artist;
    QLineEdit *album;
    QLineEdit *titre;
public:
    TagEditor(QString song);

public slots:
    void save();
};

#endif // TAGEDITOR_HPP
