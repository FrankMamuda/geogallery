/*
 * Copyright (C) 2016 Zvaigznu Planetarijs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//
// includes
//
#include <QtWidgets/QMainWindow>
#include <QtNetwork>
#include <QList>
#include <QSettings>
#include <QResizeEvent>
#include "networkrequestmanager.h"
#include "ui_mainwindow.h"
#include "imagemodel.h"
#include "imagelog.h"
#include <QHash>

//
// namespace: Ui
//
namespace Ui {
class MainWindow;
}

//
// classes
//
class ImageTableModel;

/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();
    QHash<QString, ImageLog*> imageHash;
    QMultiHash<QString, ImageLog*> cacheHash;
    QTableView *imageView;
    QString currentCacheGuid() const { return this->m_currentCacheGuid; }

private slots:
    void requestProcessed( const QString &url, NetworkRequestManager::Type type, const QVariant &userData, QByteArray data, bool error );
    int preProcessPage( const QString &data );
    void parseIntitalRequest( const QString &url, const QString &data, const QVariant &userData );
    void parseCoordInfoRequest( const QString &data );
    void stopped();
    void getImageList( const QString &data, const QString &cacheGuid );
    void readImageMetaData( const QByteArray &data, const QString &guid );
    void on_actionOpenFolder_triggered();
    void clear();
    void on_tableView_clicked( const QModelIndex &index );
    void on_tableView_doubleClicked( const QModelIndex &index );
    void on_actionOpenURL_triggered();
    void on_actionOpenGPX_triggered();
    void downloadGallery( const QString &guid, bool clear = true );
    void setCurrentCacheGuid( const QString &guid = QString::null ) { this->m_currentCacheGuid = guid; }
    void on_currentCache_currentIndexChanged( const QString &guid );
    void updateProgessBar();
    void addCacheToComboBox( const QString &guid );

    void on_actionDebug_triggered();

protected:
    virtual void resizeEvent( QResizeEvent *event );

private:
    Ui::MainWindow *ui;
    NetworkRequestManager *manager;
    QString m_currentCacheGuid;
    ImageTableModel *imageTableModel;
    bool validateURL( const QString &url );
    void addCoordInfoRequest( const QString &url );
    int m_imagesProcessed;
    int m_imagesTotal;
};

#endif // MAINWINDOW_H
