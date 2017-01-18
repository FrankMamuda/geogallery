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
#include "gpxreader.h"
#include <QDebug>
#include <QRegularexpression>

/**
 * @brief GPXReader::read
 * @param file
 * @return
 */
QStringList GPXReader::read( QFile *file ) {
    QString error;
    QDomElement root, child;
    int line, column;
    QDomDocument gpxFile;
    QStringList out;
    QString guid;

    if ( !gpxFile.setContent( file, true, &error, &line, &column )) {
        qDebug() << "GPX parse error at line" << line << "column" << column << ":" << error;
        return out;
    }

    root = gpxFile.documentElement();
    if ( QString::compare( root.tagName(), "gpx", Qt::CaseInsensitive )) {
        qDebug() << "this is not a GPX file" << root.tagName();
        return out;
    }

    child = root.firstChildElement( "wpt" );
    while ( !child.isNull()) {
        guid = GPXReader::parseWaypointElement( child );

        if ( !guid.isEmpty())
            out << guid;

        child = child.nextSiblingElement( "wpt" );
    }
    return out;
}

/**
 * @brief GPXReader::parseWaypointElement
 * @param element
 * @return
 */
QString GPXReader::parseWaypointElement( const QDomElement &element ) {
    QString url;//, cacheName;
    QRegularExpression re( "\\/seek\\/cache_details.aspx\\?guid=([a-z0-9-]+)" );
    QRegularExpressionMatch match;

    url = element.firstChildElement( "url" ).text();
    //cacheName = element.firstChildElement( "urlname" ).text();
    if ( url.isEmpty())
        return "";

    match = re.match( url );
    if ( !match.hasMatch())
        return "";

    return match.captured( 1 );
}
