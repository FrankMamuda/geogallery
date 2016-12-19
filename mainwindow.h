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
    QList<ImageLog*> imageLogList;
    QTableView *imageView;

private slots:
    void requestProcessed( const QString &url, NetworkRequestManager::Type type, QByteArray data, bool error );
    void preProcessPage( const QString &data );
    void parseIntitalRequest( const QString &data);
    void downloadImages();
    void getImageList( const QString &data );
    void readImageMetaData( const QString &url, const QByteArray &data );
    bool validateURL( const QString &url );
    void on_actionOpenFolder_triggered();
    void clear();
    void on_tableView_clicked(const QModelIndex &index);
    void on_tableView_doubleClicked(const QModelIndex &index);
    void on_actionOpenURL_triggered();

protected:
    virtual void resizeEvent( QResizeEvent *event );

private:
    Ui::MainWindow *ui;
    QStringList imageList;
    QHash<QString, QString> urlHash;
    NetworkRequestManager *manager;
    int m_totalPages;
    int m_totalImages;
    int m_imagesProcessed;
    QString m_currentURL;
    QString m_guid;
    ImageTableModel *imageTableModel;
};

#endif // MAINWINDOW_H
