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

#ifndef IMAGELOG_H
#define IMAGELOG_H

//
// includes
//
#include <QPixmap>
#include <QGeoCoordinate>
#include "networkrequestmanager.h"
#include "exifreader.h"

/**
 * @brief The ImageLog class
 */
class ImageLog : public QObject {
    Q_OBJECT

public:
    ImageLog( const QString &imageGuid, const QString &logGuid, const QString &cacheGuid, const QString &imageURL = QString(), const QString &cacheName = QString());
    QString fileName() const;
    QGeoCoordinate coords() const { return this->m_coords; }
    QPixmap thumbnail() const { return this->m_thumbnail; }
    QString imageURL() const { return this->m_imageURL; }
    QString imageGuid() const { return this->m_imageGuid; }
    QString logGuid() const { return this->m_logGuid; }
    QString cacheGuid() const { return this->m_cacheGuid; }
    QString cacheName() const { return this->m_cacheName; }
    QString logURL() const;

public slots:
    void addToDownloadQueue( NetworkRequestManager *manager );
    void setCoordinates( const QGeoCoordinate &coordinates ) { this->m_coords = coordinates; }
    void generateThumbnail();

private:
    QGeoCoordinate m_coords;
    QPixmap m_thumbnail;
    QString m_imageGuid;
    QString m_logGuid;
    QString m_cacheGuid;
    QString m_imageURL;
    QString m_cacheName;
};

#endif // IMAGELOG_H
