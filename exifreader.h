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

#ifndef EXIFREADER_H
#define EXIFREADER_H

//
// includes
//
#include <QObject>
#include <QDataStream>
#include <QByteArray>

/**
 * @brief The ExifReader class
 */
class ExifReader : public QObject {
    Q_OBJECT

public:
    static bool readGPSCoordinates( const QByteArray &data, QString &out );
    static double readRational( QDataStream &stream );

private:

    /**
     * @brief The tiffHeader_s struct
     */
    struct tiffHeader_s {
        quint16 byteOrder;
        quint16 magic;
        quint32 offset;
        quint16 numEntries;
        friend QDataStream & operator >> ( QDataStream &s, tiffHeader_s &t ) { s >> t.byteOrder >> t.magic >> t.offset >> t.numEntries; return( s ); }
    };

    /**
     * @brief The ifd0Format enum
     */
    enum ifd0Format {
        NoFormat = 0,
        ByteFormat,     // ABCD
        ASCIIFormat,    // ABCD
        ShortFormat,    // AABB
        LongFormat,     // AAAA
        RationalFormat  // AAAA/BBBB
    };

    /**
     * @brief The ifd0Tag_s struct
     */
    struct ifd0Tag_s {
        quint16 id;
        quint16 format;
        quint32 numEntries;
        quint32 data;
        friend QDataStream & operator >> ( QDataStream &s, ifd0Tag_s &t ) { s >> t.id >> t.format >> t.numEntries >> t.data; return( s ); }

        QByteArray bytes() {
            QByteArray out;
            out.append(( data & 0xff000000 ) >> 24 );
            out.append(( data & 0x00ff0000 ) >> 16 );
            out.append(( data & 0x0000ff00 ) >> 8 );
            out.append( data & 0x000000ff );
            return out;
        }
    };

    /**
     * @brief The CoordinateReference enum
     */
    enum CoordinateReference {
        NoReference = -1,
        North,
        South,
        West,
        East
    };
};

#endif // EXIFREADER_H
