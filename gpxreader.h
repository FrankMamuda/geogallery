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

#ifndef GPXREADER_H
#define GPXREADER_H

//
// includes
//
#include <QDomDocument>
#include <QFile>

/**
 * @brief The GPXTree class
 */
class GPXReader : public QObject {
    Q_OBJECT

public:
    GPXReader() {}
    static QStringList read( QFile *file );

private:
    static QString parseWaypointElement( const QDomElement &element );
};

#endif // GPXREADER_H

