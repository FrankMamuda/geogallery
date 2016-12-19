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

/**
 * @brief The ImageLog class
 */
class ImageLog : public QObject {
    Q_OBJECT

public:
    ImageLog( const QString &fileName, const QString &logURL, const QString &coords );
    QString fileName() const { return this->m_fileName; }
    QString logURL() const { return this->m_logURL; }
    QString coords() const { return this->m_coords; }
    //double latitude() const { return this->m_lat; }
    //double longitude() const { return this->m_lon; }
    QPixmap thumbnail() const { return this->m_thumbnail; }

public slots:
    void setFileName( const QString &fileName ) { this->m_fileName = fileName; }
    void setLogURL( const QString &logURL ) { this->m_logURL = logURL; }
    void setCoords( const QString &coords ) { this->m_coords = coords; }
    //void setLatitude( double latitude ) { this->m_lat = latitude; }
    //void setLongitude( double longitude ) { this->m_lat = longitude; }
    void setThumbnail( const QPixmap &thumbnail ) { this->m_thumbnail = thumbnail; }

private:
    QString m_fileName;
    QString m_logURL;
    QString m_coords;
    QPixmap m_thumbnail;
    //double m_lat;
    //double m_lon;
};

#endif // IMAGELOG_H
