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

/**
 * @brief ImageLog::ImageLog
 * @param fileName
 * @param logURL
 * @param lat
 * @param lon
 */
ImageLog::ImageLog( const QString &fileName, const QString &logURL, const QString &coords ) : m_fileName( fileName ), m_logURL( logURL ), m_coords( coords ) {
    QPixmap image( fileName );
    QRect rect;

    if ( image.width() > image.height())
        rect = QRect( image.width() / 2 - image.height() / 2, 0, image.height(), image.height());
    else if ( image.width() < image.height())
        rect = QRect( 0, image.height() / 2 - image.width() / 2, image.width(), image.width());

    this->m_thumbnail = image.copy( rect ).scaled( 128, 128, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}
