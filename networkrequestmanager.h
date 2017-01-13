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

#ifndef NETWORKREQUESTMANAGER_H
#define NETWORKREQUESTMANAGER_H

//
// includes
//
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>

//
// namespace Network
//
namespace Network {
static const unsigned int MaxRequests = 10;
}

/**
 * @brief The NetworkRequestManager class
 */
class NetworkRequestManager : public QObject {
    Q_OBJECT
    Q_ENUMS( Type )

public:
    explicit NetworkRequestManager( QObject *parent = 0 );
    ~NetworkRequestManager();
    enum Type {
        NoType = -1,
        Initial,
        HTML,
        Image,
        CoordInfo
    };

    bool isRunning() const { return this->m_running; }

    // TODO: move to PRIVATE
    QList<QNetworkRequest> activeRequests;
    QList<QNetworkRequest> requestList;

signals:
    void finished( const QString &url, NetworkRequestManager::Type type, const QVariant &userData, QByteArray data, bool error );
    void stopped();

public slots:
    void add( const QString &url, NetworkRequestManager::Type type = HTML, const QVariant &userData = QVariant(), bool priority = false );
    void execute( const QString &url, NetworkRequestManager::Type type = HTML, const QVariant &userData = QVariant(), bool priority = false ) { this->add( url, type, userData, priority ); this->run(); }
    void run();
    void stop() { this->m_running  = false; }
    void clear();

private slots:
    void replyReceived( QNetworkReply *reply );

private:

    QNetworkAccessManager *accessManager;
    bool m_running;
};

#endif // NETWORKREQUESTMANAGER_H
