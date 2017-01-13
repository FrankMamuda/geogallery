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
#include "imagemodel.h"

/**
 * @brief ImageTableModel::columnCount
 * @return
 */
int ImageTableModel::columnCount( const QModelIndex & ) const {    
    if ( !this->list.count() || this->gui == NULL )
        return 0;

    return static_cast<int>( floor( this->gui->imageView->width() / 128.0f ));
}

/**
 * @brief ImageTableModel::rowCount
 * @param index
 * @return
 */
int ImageTableModel::rowCount( const QModelIndex &index ) const {
    if ( !this->list.count())
        return 0;

    return static_cast<int>( ceil( this->list.count() / static_cast<double>( this->columnCount( index ))));
}

/**
 * @brief ImageTableModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ImageTableModel::data( const QModelIndex &index, int role ) const {
    int imageIndex;

    if ( index.isValid() && this->gui != NULL ) {
        switch ( role )  {
        case Qt::DecorationRole:
            imageIndex = index.row() * this->columnCount( index ) + index.column();
            if ( imageIndex < 0 || imageIndex >= this->list.count())
                return QVariant();

            return this->list.at( imageIndex )->thumbnail();
        }
    }

    return QVariant();
}

/**
 * @brief ImageTableModel::reset
 */
void ImageTableModel::reset() {
    this->list.clear();

    if ( this->gui == NULL )
        return;

    this->beginResetModel();
    this->list = this->gui->cacheHash.values( this->gui->currentCacheGuid());
    this->endResetModel();
}

