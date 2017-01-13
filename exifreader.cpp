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
#include "exifreader.h"
#include <QDebug>

/**
 * @brief ExifReader::readRational
 * @param stream
 * @return
 */
double ExifReader::readRational( QDataStream &stream ) {
    quint32 one, two;
    stream >> one;
    stream >> two;
    return static_cast<double>( one ) / static_cast<double>( two );
}

/**
 * @brief ExifReader::readGPSCoordinates
 * @param data
 * @param out
 * @return
 */
bool ExifReader::readGPSCoordinates( const QByteArray &data, QGeoCoordinate &coordinates ) {
    int offset, skip, gpsOffset = -1, y, latOffset = -1, lonOffset = -1;
    quint16 numGPSEntries;
    QDataStream::ByteOrder byteOrder;
    tiffHeader_s tiffHeader;
    float latRef = 0.0f, lonRef = 0.0f;
    double lat, lon;
    char coordRef;

    // read JPEG header
    if ( data.mid( 0, 2 ) != QByteArray::fromHex( "FFD8" ))
        return false;

    // find EXIF header
    offset = data.indexOf( QByteArray::fromHex( "457869660000" ));
    if ( offset == -1 )
        return false;

    offset += 6;

    // determine endianess
    if ( data.mid( offset, 2 ) == QByteArray::fromHex( "4D4D" ))
        byteOrder = QDataStream::BigEndian;
    else if ( data.mid( offset, 2 ) == QByteArray::fromHex( "4949" ))
        byteOrder = QDataStream::LittleEndian;
    else
        return false;

    // read metadata as a stream, minding endianess
    QDataStream stream( data.mid( offset, data.length()-offset-1 ));
    stream.setByteOrder( byteOrder );
    stream >> tiffHeader;
    offset = 10;

    // check TIFF magic
    if ( tiffHeader.magic != 42 )
        return false;

    // advance to IFD0 (tiffHeader.offset is always be 8?)
    if ( tiffHeader.offset > 8 )
        stream.skipRawData( tiffHeader.offset - 8 );
    offset += tiffHeader.offset - 8;

    // find GPS IFD offset from IFD0 GPS tag
    for ( y = 0; y < tiffHeader.numEntries; y++ ) {
        ifd0Tag_s tag;

        // read tag of exactly 12 bytes in size
        stream >> tag;

        // we're only interested in GPS tag
        if ( tag.id != 0x8825 )
            continue;

        // assuming offset is stored as LONG, we don't need any conversions
        if ( tag.format == ifd0Format::LongFormat )
            gpsOffset = tag.data;
    }
    offset += tiffHeader.numEntries * 12;

    // abort if GPS IFD not found
    if ( gpsOffset == - 1 )
        return false;

    // advance to GPS IDF and read entry count
    skip = gpsOffset - offset;
    stream.skipRawData( skip );
    stream >> numGPSEntries;
    offset += 2 + skip;

    for ( y = 0; y < numGPSEntries; y++ ) {
        ifd0Tag_s tag;

        // read tag of exactly 12 bytes in size
        stream >> tag;

        // decode tags
        switch ( tag.id ) {
        // latitude reference
        case 0x0001:
            coordRef = tag.bytes().at( 0 );

            if ( coordRef == 'N' )
                latRef = 1.0f;
            else if ( coordRef == 'S' )
                latRef = -1.0f;

            break;

            // latitude (offset)
        case 0x0002:
            if ( tag.format == ifd0Format::RationalFormat )
                latOffset = static_cast<int>( tag.data );

            if ( tag.numEntries != 3 )
                return false;
            break;

            // longitude reference
        case 0x0003:
            coordRef = tag.bytes().at( 0 );

            if ( coordRef == 'E' )
                lonRef = 1.0f;
            else if ( coordRef == 'W' )
                lonRef = -1.0f;

            break;

            // longitude (offset)
        case 0x0004:
            if ( tag.format == ifd0Format::RationalFormat )
                lonOffset = static_cast<int>( tag.data );

            if ( tag.numEntries != 3 )
                return false;
            break;

        default:
            ;
        }
    }
    offset += numGPSEntries * 12;

    // any useful tags found?
    if ( latRef == 0.0f || lonRef == 0.0f || latOffset == -1 || lonOffset == -1 )
        return false;

    // advance to latitude
    skip = latOffset - offset;
    stream.skipRawData( skip );
    offset += skip;

    // read latitude
    lat = ExifReader::readRational( stream ) + ExifReader::readRational( stream ) / 60.0f + ExifReader::readRational( stream ) / 3600.0f;
    lat *= latRef;
    offset += 3 * 8;

    // advance to longitude
    skip = lonOffset - offset;
    stream.skipRawData( skip );

    // read longitude
    lon = ExifReader::readRational( stream ) + ExifReader::readRational( stream ) / 60.0f + ExifReader::readRational( stream ) / 3600.0f;
    lon *= lonRef;

    // validate coordinates
    if ( lat > 60.0f || lon > 60.0f )
        return false;

    // store coordinate string
    coordinates.setLatitude( lat );
    coordinates.setLongitude( lon );

    // report success
    return true;
}
