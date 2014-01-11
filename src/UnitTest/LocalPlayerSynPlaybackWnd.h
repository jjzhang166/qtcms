#ifndef  __LOCALPLAYERSYNPLAYBACKWND_H
#define  __LOCALPLAYERSYNPLAYBACKWND_H

#include <QObject>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include "ui_TitleView.h"
#include <QString>


class LocalPlayerSynPlaybackWnd : public QWidget
{
    Q_OBJECT

public:
    LocalPlayerSynPlaybackWnd(QWidget *parent = 0);
    ~LocalPlayerSynPlaybackWnd();
    
public:
    Ui::titleview * ui;
public:
    void paintEvent( QPaintEvent * );
};
#endif   //__LOCALPLAYERSYNPLAYBACKWND_H