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
#include "imagelog.h"
#include <QDir>

/**
 * @brief ImageLog::ImageLog
 * @param imageGuid
 * @param logGuid
 * @param cacheGuid
 * @param imageURL
 */
ImageLog::ImageLog( const QString &imageGuid, const QString &logGuid, const QString &cacheGuid, const QString &imageURL ) : m_imageGuid( imageGuid ), m_logGuid( logGuid ), m_cacheGuid( cacheGuid ), m_imageURL( imageURL ) {
    /* do nothing for now*/
}

/**
 * @brief ImageLog::generateThumbnail
 */
void ImageLog::generateThumbnail() {
    QPixmap image( this->fileName());
    QRect rect;

    if ( image.width() > image.height())
        rect = QRect( image.width() / 2 - image.height() / 2, 0, image.height(), image.height());
    else if ( image.width() < image.height())
        rect = QRect( 0, image.height() / 2 - image.width() / 2, image.width(), image.width());

    this->m_thumbnail = image.copy( rect ).scaled( 128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

/**
 * @brief ImageLog::addToDownloadQueue
 * @param manager
 */
void ImageLog::addToDownloadQueue( NetworkRequestManager *manager ) {
    manager->add( this->imageURL(), NetworkRequestManager::Image, this->imageGuid());
}

/**
 * @brief ImageLog::logURL
 * @return
 */
QString ImageLog::logURL() const {
    return QString( "https://www.geocaching.com/seek/log.aspx?LUID=%1" ).arg( this->logGuid());
}

/**
 * @brief ImageLog::fileName
 * @return
 */
QString ImageLog::fileName() const {
    return QString( "%1/cache/%2/%3_%4.jpg" ).arg( QDir::currentPath()).arg( this->cacheGuid()).arg( this->logGuid()).arg( this->imageGuid());
}
