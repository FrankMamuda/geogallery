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

//
// includes
//
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include "exifreader.h"
#include <QDesktopServices>
#include <QFileDialog>
#include "openurldialog.h"

//
// TODO: lock gui while download/parse operations pending?
//

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ), m_totalPages( 1 ) {
    ui->setupUi( this );

    // set up manager
    this->manager = new NetworkRequestManager( this );
    this->connect( this->manager, SIGNAL( finished( QString, NetworkRequestManager::Type, QByteArray, bool )), SLOT( requestProcessed( QString, NetworkRequestManager::Type, QByteArray, bool )));
    this->connect( this->manager, SIGNAL( stopped()), SLOT( downloadImages()));

    // set up view
    this->imageView = this->ui->tableView;
    this->imageTableModel = new ImageTableModel( this );
    this->imageView->setModel( this->imageTableModel );

    // hide progress bar & coord view
    this->ui->progressBar->setVisible( false );
    this->ui->coordsDisplay->setVisible( false );
}

/**
 * @brief preProcessPage
 */
void MainWindow::preProcessPage( const QString &data ) {
    QStringList list;
    QRegExp rx( "Page: <b>\\d+<\\/b> of <b>(\\d+)<\\/b>" );
    int pos = 0;
    rx.setMinimal( true );

    if ( !data.contains( "Gallery Images" )) {
        qDebug() << "requested url does not point to a geocaching gallery";
        this->m_totalPages = 0;
        return;
    }

    while (( pos = rx.indexIn( data, pos )) != -1 ) {
        list << rx.cap( 1 );
        pos += rx.matchedLength();
    }

    if ( !list.isEmpty())
        this->m_totalPages = list.first().toInt();
}

/**
 * @brief parseIntitalRequest
 */
void MainWindow::parseIntitalRequest( const QString &data ) {
    int y;

    // get number of pages in gallery
    this->preProcessPage( data );

    // get image urls
    for ( y = 1; y <= this->m_totalPages; y++ )
        this->manager->add( QString( "%1&page=%2" ).arg( this->m_currentURL ).arg( y ), NetworkRequestManager::HTML );

    this->manager->run();
}

/**
 * @brief MainWindow::getImageList
 */
void MainWindow::getImageList( const QString &data ) {
    QRegExp rx( "quot;(https:[\\/.a-z]+log.aspx\\?LUID=[a-z0-9-&A-Z=]+)&quot;.+\\n.+<img\\s+src='(https:[\\/.a-z0-9-]+)'" );
    int pos = 0;
    rx.setMinimal( true );

    while (( pos = rx.indexIn( data, pos )) != -1 ) {
        // fetch full sized images instead of thumbnails
        QString imgURL = rx.cap( 2 ).replace( "thumb", "large" );
        this->imageList << imgURL;

        // get log url
        this->urlHash[imgURL.mid( imgURL.length()-40, 36 )] = rx.cap( 1 );

        // advance
        pos += rx.matchedLength();
    }

    // show progress bar
    this->ui->progressBar->setVisible( true );
    this->ui->progressBar->setValue( 0 );
}

/**
 * @brief MainWindow::requestProcessed
 * @param url
 * @param type
 * @param data
 * @param error
 */
void MainWindow::requestProcessed( const QString &url, NetworkRequestManager::Type type, QByteArray data, bool error ) {
    if ( error ) {
        qDebug() << "MainWindow::requestProcessed: error";
        return;
    }

    switch ( type ) {
    case NetworkRequestManager::Initial:
        this->parseIntitalRequest( data );
        break;

    case NetworkRequestManager::HTML:
        this->getImageList( data );
        break;

    case NetworkRequestManager::Image:
        this->readImageMetaData( url, data );
        break;

    case NetworkRequestManager::NoType:
    default:
        qDebug() << "MainWindow::requestProcessed: unknown request type";
        return;
    }
}

/**
 * @brief MainWindow::downloadImages
 */
void MainWindow::downloadImages() {
    if ( this->imageList.isEmpty()) {
        if ( !this->imageLogList.isEmpty()) {
            this->imageTableModel->reset();
            this->urlHash.clear();
            this->ui->progressBar->setVisible( false );
        }
        return;
    }

    foreach ( QString imageURL, this->imageList )
        this->manager->add( imageURL, NetworkRequestManager::Image );

    this->m_totalImages = this->imageList.count();
    this->imageList.clear();
    this->manager->run();
}

/**
 * @brief MainWindow::readImageMetaData
 * @param data
 */
void MainWindow::readImageMetaData( const QString &url, const QByteArray &data ) {
    this->m_imagesProcessed++;
    this->ui->progressBar->setValue( static_cast<int>( static_cast<float>( this->m_imagesProcessed ) / static_cast<float>( this->m_totalImages ) * 100.0f ));

    // get exif data, first 65536 bytes should be ok
    QString coords;
    if ( ExifReader::readGPSCoordinates( data, coords )) {
        // get cache directory
        QFileInfo fileInfo( url );
        QDir dir( QString( "%1/cache/%2/" ).arg( QDir::currentPath()).arg( this->m_guid ));
        if ( !dir.exists())
            dir.mkpath( "." );

        // store cached geodatgged image
        QFile file( QString( "%1/cache/%2/%3" ).arg( QDir::currentPath()).arg( this->m_guid ).arg( fileInfo.fileName()));
        if ( file.open( QFile::WriteOnly )) {
            file.write( data );
            file.close();
        }

        // add a new log
        this->imageLogList << new ImageLog( file.fileName(), this->urlHash[url.mid( url.length()-42, 36 )], coords );
    }
}

/**
 * @brief MainWindow::on_actionOpenFolder_triggered
 */
void MainWindow::on_actionOpenFolder_triggered() {
    QString dir = QFileDialog::getExistingDirectory( this, this->tr( "Open Directory" ), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    // show progress bar
    this->ui->progressBar->setVisible( true );

    if ( !dir.isEmpty()) {
        this->clear();

        // get file list
        QStringList filter( "*.jpg" );
        QDir directory( dir );
        QStringList jpgList = directory.entryList( filter );
        this->m_totalImages = jpgList.count();

        // check every jpeg
        foreach ( QString jpg, jpgList ) {
            QFile file( dir + "/" + jpg );

            // update progessbar
            this->m_imagesProcessed++;
            this->ui->progressBar->setValue( static_cast<int>( static_cast<float>( this->m_imagesProcessed ) / static_cast<float>( this->m_totalImages ) * 100.0f ));

            if ( file.open( QFile::ReadOnly )) {
                // get exif data, first 65536 bytes should be ok
                QString coords;
                if ( ExifReader::readGPSCoordinates( file.read( 65536 ), coords ))
                    this->imageLogList << new ImageLog( file.fileName(), "", coords );

                file.close();
            }

        }
        this->imageTableModel->reset();
    }

    // hide progress bar
    this->ui->progressBar->setVisible( false );

    // show coordinate display
    this->ui->coordsDisplay->setVisible( true );
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow() {
    qDeleteAll( this->imageLogList );
    delete this->imageTableModel;
    this->manager->deleteLater();
    delete ui;
}

/**
 * @brief MainWindow::validateURL
 * @param url
 */
bool MainWindow::validateURL( const QString &url ) {
    QStringList list;
    QRegExp rx( "https:\\/\\/www.geocaching.com\\/seek\\/gallery.aspx\\?guid=([a-z0-9]+-[a-z0-9]+-[a-z0-9]+-[a-z0-9]+-[a-z0-9]+)" );
    int pos = 0;
    rx.setMinimal( true );

    if ( url.isEmpty())
        return false;

    while (( pos = rx.indexIn( url, pos )) != -1 ) {
        list << rx.cap( 1 );
        pos += rx.matchedLength();
    }

    if ( list.isEmpty())
        return false;

    // store guid
    this->m_guid = list.first();

    return true;
}

/**
 * @brief MainWindow::clear
 */
void MainWindow::clear() {
    this->m_currentURL = "";
    this->m_totalPages = 0;
    this->m_totalImages = 0;
    this->m_imagesProcessed = 0;
    this->m_guid = "";
    this->imageList.clear();
    this->urlHash.clear();
    this->ui->coordsDisplay->clear();
    this->ui->coordsDisplay->setVisible( false );
    qDeleteAll( this->imageLogList );
    this->imageLogList.clear();
}

/**
 * @brief MainWindow::resizeEvent
 * @param event
 */
void MainWindow::resizeEvent( QResizeEvent *event ) {
    this->imageTableModel->reset();
    event->accept();
}

/**
 * @brief MainWindow::on_tableView_clicked
 * @param index
 */
void MainWindow::on_tableView_clicked( const QModelIndex &index ) {
    int imageIndex = index.row() * this->imageTableModel->columnCount( index ) + index.column();
    if ( imageIndex < 0 || imageIndex >= this->imageLogList.count()) {
        this->ui->coordsDisplay->clear();
        return;
    }

    this->ui->coordsDisplay->setText( this->imageLogList.at( imageIndex )->coords());
}

/**
 * @brief MainWindow::on_tableView_doubleClicked
 * @param index
 */
void MainWindow::on_tableView_doubleClicked(const QModelIndex &index) {
    int imageIndex = index.row() * this->imageTableModel->columnCount( index ) + index.column();
    if ( imageIndex < 0 || imageIndex >= this->imageLogList.count()) {
        this->ui->coordsDisplay->clear();
        return;
    }

    ImageLog *imageLog = this->imageLogList.at( imageIndex );
    if ( !imageLog->logURL().isEmpty())
        QDesktopServices::openUrl( QUrl( imageLog->logURL()));
    else
        QDesktopServices::openUrl( QUrl::fromLocalFile( imageLog->fileName()));
}

/**
 * @brief MainWindow::on_actionOpenURL_triggered
 */
void MainWindow::on_actionOpenURL_triggered() {
    OpenURLDialog dialog;
    this->m_currentURL = dialog.ui->urlEdit->toPlainText();

    switch ( dialog.exec()) {
    case QDialog::Accepted:
        if ( this->validateURL( this->m_currentURL )) {
            this->manager->clear();
            this->manager->add( this->m_currentURL, NetworkRequestManager::Initial );
            this->manager->run();

            // show coordinate display
            this->ui->coordsDisplay->setVisible( true );
        } else {
            QMessageBox msgBox;
            msgBox.setText( this->tr( "Invalid url" ));
            msgBox.setIcon( QMessageBox::Warning );
            msgBox.exec();
        }
        break;

    default:
        break;
    }
}
