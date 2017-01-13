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
#include <QRegularExpression>
#include "gpxreader.h"
#include "main.h"

//
// TODO: lock gui while download/parse operations pending?
//

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::MainWindow ) {
    ui->setupUi( this );

    // set up manager
    this->manager = new NetworkRequestManager( this );
    this->connect( this->manager, SIGNAL( finished( QString, NetworkRequestManager::Type, QVariant, QByteArray, bool )), SLOT( requestProcessed( QString, NetworkRequestManager::Type, QVariant, QByteArray, bool )));
    this->connect( this->manager, SIGNAL( stopped()), SLOT( stopped()));

    // set up view
    this->imageView = this->ui->tableView;
    this->imageTableModel = new ImageTableModel( this );
    this->imageView->setModel( this->imageTableModel );

    // hide progress bar & coord view
    this->ui->progressBar->setVisible( false );
    this->ui->coordsDisplay->setVisible( false );
}

/**
 * @brief MainWindow::preProcessPage
 * @param data
 * @return
 */
int MainWindow::preProcessPage( const QString &data ) {
    QRegularExpression re( "Page: <b>\\d+<\\/b> of <b>(\\d+)<\\/b>" );
    QRegularExpressionMatch match;

    // invalid gallery
    if ( !data.contains( "Gallery Images" )) {
        qDebug() << "requested url does not point to a geocaching gallery";
        return -1;
    }

    // determine number of pages
    match = re.match( data );
    if ( !match.hasMatch())
        return 0;

    // process gallery
    return match.captured( 1 ).toInt();
}

/**
 * @brief MainWindow::parseIntitalRequest
 * @param url
 * @param data
 * @param userData
 */
void MainWindow::parseIntitalRequest( const QString &url, const QString &data, const QVariant &userData ) {
    int y, numPages;

    // get number of pages in gallery
    numPages = this->preProcessPage( data );
    if ( numPages < 0 )
        return;

    // get image urls (within reasonable limits - max 250)
    if ( numPages > 0 ) {
        if ( numPages > Main::MaxGalleryPages )
            numPages = Main::MaxGalleryPages;

        // download each and every gallery page
        for ( y = 1; y <= numPages; y++ )
            this->manager->add( QString( "%1&page=%2" ).arg( url ).arg( y ), NetworkRequestManager::HTML, userData );

        this->manager->run();
    } else {
        // single page galleries can be processed directly
        this->getImageList( data, userData.toString());
    }
}

/**
 * @brief MainWindow::getImageList
 * @param data
 * @param cacheGuid
 */
void MainWindow::getImageList( const QString &data, const QString &cacheGuid ) {
    QRegExp rx( "quot;https:[\\/.a-z]+log.aspx\\?LUID=([a-z0-9-]+).+\\n.+<img\\s+src='(https:[\\/.a-z0-9-]+)'" );
    QRegularExpression re( "https:\\/\\/img.geocaching.com\\/cache\\/log\\/[a-z]+\\/([a-z0-9-]+)." );
    QRegularExpressionMatch match;
    int pos = 0;
    ImageLog *imageLog;
    QString imageURL, imageGuid, logGuid;

    // go through image list and create image logs
    rx.setMinimal( true );
    while (( pos = rx.indexIn( data, pos )) != -1 ) {
        // fetch full sized images instead of thumbnails
        imageURL = rx.cap( 2 ).replace( "thumb", "large" );

        // get image and log guids
        match = re.match( imageURL );
        if ( !match.hasMatch()) {
            pos += rx.matchedLength();
            continue;
        }
        imageGuid = match.captured( 1 );
        logGuid = rx.cap( 1 );

        // add new image log
        imageLog = new ImageLog( imageGuid, logGuid, cacheGuid, imageURL );
        imageLog->addToDownloadQueue( this->manager );
        this->imageHash[imageGuid] = imageLog;

        // advance
        pos += rx.matchedLength();
        this->m_imagesTotal++;
    }

    // DELETEME: debug
    qDebug() << "imageList" << this->imageHash.count();

    // execute queued requests
    this->manager->run();
}

/**
 * @brief MainWindow::requestProcessed
 * @param url
 * @param type
 * @param userData
 * @param data
 * @param error
 */
void MainWindow::requestProcessed( const QString &url, NetworkRequestManager::Type type, const QVariant &userData, QByteArray data, bool error ) {
    if ( error ) {
        qDebug() << "MainWindow::requestProcessed: error";
        return;
    }

    switch ( type ) {
    case NetworkRequestManager::Initial:
        this->parseIntitalRequest( url, data, userData );
        break;

    case NetworkRequestManager::HTML:
        this->getImageList( data, userData.toString());
        break;

    case NetworkRequestManager::Image:
        this->readImageMetaData( data, userData.toString());
        break;

    case NetworkRequestManager::CoordInfo:
        this->parseCoordInfoRequest( data );
        break;

    case NetworkRequestManager::NoType:
    default:
        qDebug() << "MainWindow::requestProcessed: unknown request type";
        return;
    }

    //if ( this->manager->isRunning())
    //    this->manager->run();
}

/**
 * @brief MainWindow::stopped
 */
void MainWindow::stopped() {
    /* do nothing for now*/
}

/**
 * @brief MainWindow::readImageMetaData
 * @param data
 * @param guid
 */
void MainWindow::readImageMetaData( const QByteArray &data, const QString &guid ) {
    ImageLog *imageLog;
    QGeoCoordinate coordinates;

    // find image
    imageLog = this->imageHash[guid];
    if ( imageLog == NULL )
        return;

    // update
    this->m_imagesProcessed++;
    this->updateProgessBar();

    // get exif data
    if ( ExifReader::readGPSCoordinates( data, coordinates )) {
        // get cache directory
        QFile file( imageLog->fileName());
        QFileInfo info( file );
        QDir dir( info.absolutePath());
        if ( !dir.exists())
            dir.mkpath( "." );

        // store cached geodatgged image
        if ( file.open( QFile::WriteOnly )) {
            file.write( data );
            file.close();
        }

        // store coordinates
        imageLog->setCoordinates( coordinates );
        imageLog->generateThumbnail();

        // DELETEME: debug
        qDebug() << "hit for" << imageLog->cacheGuid();

        // add image to display list
        this->cacheHash.insert( imageLog->cacheGuid(), imageLog );

        this->addCacheToComboBox( imageLog->cacheGuid());

        this->imageTableModel->reset();
    } else {
        this->imageHash.remove( guid );
    }
}

/**
 * @brief MainWindow::updateProgessBar
 */
void MainWindow::updateProgessBar() {
    int value;

    // processedImages / totalImages
    value = static_cast<int>( static_cast<float>( this->m_imagesProcessed ) / static_cast<float>( this->m_imagesTotal ) * 100.0f );

    // hide progress bar when all done
    if ( value == 100 )
        this->ui->progressBar->hide();
    else
        this->ui->progressBar->setValue( value );
}

/**
 * @brief MainWindow::on_actionOpenFolder_triggered
 */
void MainWindow::on_actionOpenFolder_triggered() {
    QString dir;
    QRegularExpression reDir( "([a-z0-9]{8}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{12})" );
    QRegularExpression reImage( "([a-z0-9]{8}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{12})_([a-z0-9]{8}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{4}-[a-z0-9]{12}).jpg" );
    QRegularExpressionMatch match;
    QStringList jpgList;

    // get directory
    dir = QFileDialog::getExistingDirectory( this, this->tr( "Open Directory" ), QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

    // show progress bar
    this->m_imagesProcessed = 0;
    this->ui->progressBar->setVisible( true );

    if ( !dir.isEmpty()) {
        // get file list
        QStringList filter( "*.jpg" );
        QDir directory( dir );

        this->clear();
        jpgList = directory.entryList( filter );
        this->m_imagesTotal = jpgList.count();

        // check every jpeg
        foreach ( QString jpg, jpgList ) {
            QString imageGuid, cacheGuid, logGuid;
            QFile file( dir + "/" + jpg );

            // update progessbar
            this->m_imagesProcessed++;
            this->updateProgessBar();

            // get cache guid
            match = reDir.match( directory.absolutePath());
            if ( !match.hasMatch())
                continue;
            else
                cacheGuid = match.captured( 1 );

            // get log and image guid
            match = reImage.match( jpg );
            if ( !match.hasMatch())
                continue;
            else {
                logGuid = match.captured( 1 );
                imageGuid = match.captured( 2 );
            }

            // abort on invalid guids
            if ( cacheGuid.isEmpty() || imageGuid.isEmpty() || logGuid.isEmpty())
                continue;

            // read coordinates
            if ( file.open( QFile::ReadOnly )) {
                // get exif data, first 65536 bytes should be ok
                QGeoCoordinate coordinates;
                if ( ExifReader::readGPSCoordinates( file.read( 65536 ), coordinates )) {
                    ImageLog *imageLog;

                    // constuct a new image log
                    imageLog = new ImageLog( imageGuid, logGuid, cacheGuid, "" );
                    imageLog->setCoordinates( coordinates );
                    imageLog->generateThumbnail();

                    // set current cache guid and add it to combo box
                    this->setCurrentCacheGuid( cacheGuid );
                    this->addCacheToComboBox( cacheGuid );

                    // add to display list
                    this->cacheHash.insert( cacheGuid, imageLog );
                }
                file.close();
            }
        }
        // update view
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
    qDeleteAll( this->imageHash );
    delete this->imageTableModel;
    this->manager->deleteLater();
    delete ui;
}

/**
 * @brief MainWindow::addCoordInfoRequest
 * @param url
 */
void MainWindow::addCoordInfoRequest( const QString &url ) {
    this->manager->execute( url, NetworkRequestManager::CoordInfo );
}

/**
 * @brief MainWindow::parseCoordInfoRequest
 * @param data
 */
void MainWindow::parseCoordInfoRequest( const QString &data ) {
    QRegularExpression re( "\\/seek\\/cache_details.aspx\\?guid=([a-z0-9-]+)" );
    QRegularExpressionMatch match;

    if ( data.isEmpty())
        return;

    match = re.match( data );
    if ( !match.hasMatch())
        return;

    // process gallery
    this->downloadGallery( match.captured( 1 ));
}

/**
 * @brief MainWindow::addCacheToComboBox
 */
void MainWindow::addCacheToComboBox( const QString &guid ) {
    int y, found = false;

    for ( y = 0; y < this->ui->currentCache->count(); y++ ) {
        if ( !QString::compare( guid, this->ui->currentCache->itemText( y ))) {
            found = true;
            break;
        }
    }
    if ( !found )
        this->ui->currentCache->insertItem( 0, guid );
}

/**
 * @brief MainWindow::downloadGallery
 * @param guid
 * @param clear
 */
void MainWindow::downloadGallery( const QString &guid, bool clear ) {
    // clear view and network manager
    if ( clear ) {
        this->clear();
        this->manager->clear();
    }

    // add cache guid to list
    //this->addCacheToComboBox( guid );

    // begin fetching images
    this->manager->execute( "https://www.geocaching.com/seek/gallery.aspx?guid=" + guid, NetworkRequestManager::Initial, guid );

    // show coordinate display
    if ( !this->ui->coordsDisplay->isVisible())
        this->ui->coordsDisplay->setVisible( true );

    // show progress bar
    if ( !this->ui->progressBar->isVisible())
        this->ui->progressBar->setVisible( true );
}

/**
 * @brief MainWindow::validateURL
 * @param url
 * @return
 */
bool MainWindow::validateURL( const QString &url ) {
    QRegularExpression re( "\\/seek\\/gallery.aspx\\?guid=([a-z0-9-]+)" );
    QRegularExpressionMatch match;

    // abort on empty coords
    if ( url.isEmpty())
        return false;

    // parse coord.info requests
    if ( url.contains( "coord.info" )) {
        // avoid SSL
        QString coordInfoURL = url;
        coordInfoURL.replace( "https", "http" );
        this->addCoordInfoRequest( coordInfoURL );
        return true;
    }

    match = re.match( url );
    if ( !match.hasMatch())
        return false;

    // process gallery
    this->downloadGallery( match.captured( 1 ));

    // return success
    return true;
}

/**
 * @brief MainWindow::clear
 */
void MainWindow::clear() {
    this->imageHash.clear();
    this->cacheHash.clear();
    qDeleteAll( this->imageHash );
    this->setCurrentCacheGuid();
    this->ui->currentCache->clear();
    this->ui->coordsDisplay->clear();
    this->ui->coordsDisplay->setVisible( false );
    this->ui->coordsDisplay->setVisible( false );
    this->ui->progressBar->setVisible( false );
    this->ui->progressBar->setValue( 0 );
    this->m_imagesProcessed = 0;
    this->m_imagesTotal = 0;
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
    ImageLog *imageLog;
    int imageIndex;

    // get image index from table
    imageIndex = index.row() * this->imageTableModel->columnCount( index ) + index.column();
    if ( imageIndex < 0 || imageIndex >= this->imageTableModel->list.count()) {
        this->ui->coordsDisplay->clear();
        return;
    }

    // get corresponding image log
    imageLog = this->imageTableModel->list.at( imageIndex );

    // display gps coordinates
    if ( imageLog != NULL )
        this->ui->coordsDisplay->setText( imageLog->coords().toString( QGeoCoordinate::DegreesMinutesWithHemisphere ));
}

/**
 * @brief MainWindow::on_tableView_doubleClicked
 * @param index
 */
void MainWindow::on_tableView_doubleClicked( const QModelIndex &index ) {
    ImageLog *imageLog;
    int imageIndex;

    // get image index from table
    imageIndex = index.row() * this->imageTableModel->columnCount( index ) + index.column();
    if ( imageIndex < 0 || imageIndex >= this->imageTableModel->list.count()) {
        this->ui->coordsDisplay->clear();
        return;
    }

    // get corresponding image log
    imageLog = this->imageTableModel->list.at( imageIndex );

    // open log page
    if ( imageLog != NULL )
        QDesktopServices::openUrl( QUrl( imageLog->logURL()));
}

/**
 * @brief MainWindow::on_actionOpenURL_triggered
 */
void MainWindow::on_actionOpenURL_triggered() {
    OpenURLDialog dialog;
    QString url;

    switch ( dialog.exec()) {
    case QDialog::Accepted:
        url = dialog.ui->urlEdit->toPlainText();
        if ( !this->validateURL( url )) {
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

/**
 * @brief MainWindow::on_actionOpenGPX_triggered
 */
void MainWindow::on_actionOpenGPX_triggered() {
    QString fileName;
    QStringList guidList;

    // get gpx filename
    fileName = QFileDialog::getOpenFileName( this, this->tr( "Open GPX file" ), QDir::currentPath(), this->tr( "GPX files (*.gpx)" ));

    // parse gpx and get cache guid list
    QFile file( fileName );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text )) {
        guidList = GPXReader::read( &file );
        file.close();

        // download galleries
        if ( !guidList.isEmpty()) {
            this->clear();
            this->manager->clear();
            foreach ( QString guid, guidList )
                this->downloadGallery( guid, false );

            this->manager->run();
        }
    }
}

/**
 * @brief MainWindow::on_currentCache_currentIndexChanged
 * @param guid
 */
void MainWindow::on_currentCache_currentIndexChanged( const QString &guid ) {
    this->setCurrentCacheGuid( guid );
    this->ui->coordsDisplay->clear();
    this->imageTableModel->reset();
}

/**
 * @brief MainWindow::on_actionDebug_triggered
 */
void MainWindow::on_actionDebug_triggered() {
    qDebug() << "activeReq" << this->manager->activeRequests.count();
    foreach ( QNetworkRequest req, this->manager->activeRequests )
        qDebug() << "  url:" << req.url();
    qDebug() << "run" << this->manager->isRunning();

   // this->manager->run();
    //QList values = this->cacheHash.keys()


}
