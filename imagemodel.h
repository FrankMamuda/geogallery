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

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

//
// includes
//
#include <QAbstractTableModel>
#include <QPixmap>
#include "mainwindow.h"

//
// classes
//
class MainWindow;
class ImageLog;

/**
 * @brief The ImageTableModel class
 */
class ImageTableModel : public QAbstractTableModel {
public:
    explicit ImageTableModel( MainWindow *gui ) { this->gui = gui; }
    int rowCount( const QModelIndex &parent ) const;
    int columnCount( const QModelIndex &parent ) const;
    QVariant data( const QModelIndex &index, int role ) const;
    void reset();
    QList<ImageLog *>list;

private:
    MainWindow *gui;
};

#endif // IMAGEMODEL_H
