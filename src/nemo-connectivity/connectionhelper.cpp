/* Copyright (c) 2019 - 2021 Open Mobile Platform LLC.
 * Copyright (c) 2013 - 2019 Jolla Ltd.
 *
 * License: BSD
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#include "connectionhelper.h"

#include <QTimer>
#include <QUrl>
#include <QString>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingCallWatcher>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <connman-qt6/networkmanager.h>
#include <connman-qt6/networktechnology.h>
#include <connman-qt6/networkservice.h>
#else
#include <connman-qt5/networkmanager.h>
#include <connman-qt5/networktechnology.h>
#include <connman-qt5/networkservice.h>
#endif

class ConnectionHelperPrivate
{
public:
    ConnectionHelperPrivate();

    QTimer m_timeoutTimer;
    QNetworkAccessManager *m_networkAccessManager;
    QString m_defaultNetworkStatusCheckUrl;
    bool m_networkConfigReady;
    bool m_delayedNetworkStatusCheckUrl;
    bool m_delayedAttemptToConnect;
    bool m_detectingNetworkConnection;
    bool m_connmanIsAvailable;
    bool m_selectorVisible;
    Nemo::ConnectionHelper::Status m_status;

    NetworkManager *m_netman;

    QDBusInterface *m_connectionSelectorInterface;
};

ConnectionHelperPrivate::ConnectionHelperPrivate()
    : m_networkAccessManager(0)
    , m_networkConfigReady(false)
    , m_delayedNetworkStatusCheckUrl(false)
    , m_delayedAttemptToConnect(false)
    , m_detectingNetworkConnection(false)
    , m_connmanIsAvailable(false)
    , m_selectorVisible(false)
    , m_status(Nemo::ConnectionHelper::Offline)
    , m_netman(NetworkManagerFactory::createInstance())
    , m_connectionSelectorInterface(nullptr)
{
}

namespace Nemo {

ConnectionHelper::ConnectionHelper(QObject *parent)
    : QObject(parent)
    , d_ptr(new ConnectionHelperPrivate)
{
    connect(&d_ptr->m_timeoutTimer, &QTimer::timeout, this, &ConnectionHelper::emitFailureIfNeeded);
    d_ptr->m_timeoutTimer.setSingleShot(true);

    connect(d_ptr->m_netman, &NetworkManager::availabilityChanged, this, &ConnectionHelper::connmanAvailableChanged);
    connect(d_ptr->m_netman, &NetworkManager::stateChanged, this, &ConnectionHelper::networkStateChanged);

    if (d_ptr->m_netman->defaultRoute()) {
        if (d_ptr->m_netman->defaultRoute()->state() == "online") {
            updateStatus(ConnectionHelper::Online);
        } else if (d_ptr->m_netman->defaultRoute()->state() == "ready") {
            updateStatus(ConnectionHelper::Connected);
        }
    }

    d_ptr->m_connmanIsAvailable = QDBusConnection::systemBus().interface()->isServiceRegistered("net.connman");
    if (d_ptr->m_connmanIsAvailable) {
        determineDefaultNetworkStatusCheckUrl();
    } else {
        d_ptr->m_delayedNetworkStatusCheckUrl = true;
    }
}

ConnectionHelper::~ConnectionHelper()
{
    delete d_ptr;
}

void ConnectionHelper::determineDefaultNetworkStatusCheckUrl()
{
    d_ptr->m_delayedNetworkStatusCheckUrl = false;
    QDBusConnection::systemBus().callWithCallback(
            QDBusMessage::createMethodCall(QStringLiteral("net.connman"),
                                           QStringLiteral("/"),
                                           QStringLiteral("net.connman.Manager"),
                                           QStringLiteral("GetProperties")),
            this,
            SLOT(getConnmanManagerProperties(QVariantMap)));
}

void ConnectionHelper::getConnmanManagerProperties(const QVariantMap &props)
{
    d_ptr->m_defaultNetworkStatusCheckUrl = props.value(QStringLiteral("Ipv4StatusUrl")).toString();
    if (d_ptr->m_delayedAttemptToConnect) {
        d_ptr->m_delayedAttemptToConnect = false;
        attemptToConnectNetwork();
    }
}

void ConnectionHelper::connmanAvailableChanged(bool available)
{
    if (available) {
        d_ptr->m_networkConfigReady = true;
        if (d_ptr->m_delayedNetworkStatusCheckUrl) {
            determineDefaultNetworkStatusCheckUrl();
        } else if (d_ptr->m_delayedAttemptToConnect) {
            d_ptr->m_delayedAttemptToConnect = false;
            attemptToConnectNetwork();
        }
    }
    d_ptr->m_connmanIsAvailable = available;
}

bool ConnectionHelper::online() const
{
    return d_ptr->m_status == ConnectionHelper::Online;
}

bool ConnectionHelper::selectorVisible() const
{
    return d_ptr->m_selectorVisible;
}

ConnectionHelper::Status ConnectionHelper::status() const
{
    return d_ptr->m_status;
}

void ConnectionHelper::setSelectorVisible(bool selectorVisible)
{
    if (d_ptr->m_selectorVisible != selectorVisible) {
        d_ptr->m_selectorVisible = selectorVisible;
        emit selectorVisibleChanged();
    }
}

void ConnectionHelper::updateStatus(ConnectionHelper::Status status)
{
    if (d_ptr->m_status != status) {
        const ConnectionHelper::Status oldStatus = d_ptr->m_status;
        d_ptr->m_status = status;
        emit statusChanged();
        if (oldStatus == ConnectionHelper::Online
                || status == ConnectionHelper::Online) {
            emit onlineChanged();
        }
    }
}

/*
    Attempts to perform a network request.
    If it succeeds, the user has connected to a valid network.
    If it fails, the user has explicitly denied the network request,
    or the network connection chosen by the user was unable to be
    brought online.

    Note that if no valid network configuration exists, the user will
    be prompted to add a valid network configuration (e.g. connect to wlan).

    If the user rejects the dialog, the connection helper will
    emit networkConnectivityUnavailable(). If the user does add a valid
    configuration, a status check network request will be sent to ensure
    that the connection is online.  If that request fails or times out,
    networkConnectivityUnavailable() will be emitted; otherwise
    networkConnectivityEstablished() will be emitted.
*/
void ConnectionHelper::attemptToConnectNetwork()
{
    _attemptToConnectNetwork(true);
}

/*
    Same as attemptToConnectNetwork() function, but does not prompt
    for user to add a valid network configuration.
*/
void ConnectionHelper::requestNetwork()
{
    _attemptToConnectNetwork(false);
}

void ConnectionHelper::_attemptToConnectNetwork(bool explicitAttempt)
{
    if (!d_ptr->m_connmanIsAvailable) {
        d_ptr->m_delayedAttemptToConnect = true;
        return;
    }

    if (d_ptr->m_detectingNetworkConnection) {
        return; // already busy connecting.
    }

    // set up a timeout error emission trigger after some time,
    // unless we manage to connect in the meantime.
    // for explicit (UI-driven) flow, we will show the connection
    // selector dialog, etc, so allow plenty of time.
    d_ptr->m_detectingNetworkConnection = true;
    updateStatus(ConnectionHelper::Connecting);
    d_ptr->m_timeoutTimer.start(explicitAttempt ? 300000 : 5000); // 5 min/sec

    if (!d_ptr->m_netman->defaultRoute()) {
        emitFailureIfNeeded();
    } else if (d_ptr->m_netman->defaultRoute()->state() == "online") {
        // we are online and connman's online check has passed. Everything is ok
        handleNetworkEstablished();
    } else if (explicitAttempt) {
        // even if we are in "ready" state (i.e. possibly able to connect, but possibly
        // not, due to captive portal), immediately show the connection selector UI.
        // we do this to avoid performing a network request which might have
        // significant latency, in the "explicit" (i.e. UI triggered) case.
        openConnectionDialog();
    } else if (d_ptr->m_netman->defaultRoute()->state() == "ready") {
        // we already have an open session, but something isn't quite right.  Ensure that the
        // connection is usable (not blocked by a Captive Portal).
        performRequest();
    } else {
        // don't have a valid connection, and should not prompt user to select connection.
        emitFailureIfNeeded();
    }
}

void ConnectionHelper::performRequest()
{
    // sometimes connman service can be in 'ready' state but can still be used.
    // In this case, let perform our own online check, just to be sure

    if (!d_ptr->m_networkAccessManager) {
        d_ptr->m_networkAccessManager = new QNetworkAccessManager(this);
    }

    // Testing network connectivity, always load from network.
    const QUrl networkStatusCheckUrl(d_ptr->m_defaultNetworkStatusCheckUrl);
    QNetworkRequest request(networkStatusCheckUrl);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::AlwaysNetwork);
    QNetworkReply *reply = d_ptr->m_networkAccessManager->head(request);
    if (!reply) {
        QMetaObject::invokeMethod(this, "handleNetworkUnavailable", Qt::QueuedConnection);
        return;
    }

    // We expect this request to succeed if the connection has been brought
    // online successfully.  It may fail if, for example, the interface is waiting
    // for a Captive Portal redirect, in which case we should consider network
    // connectivity to be unavailable (as it requires user intervention).
    connect(reply, &QNetworkReply::finished, this, &ConnectionHelper::handleCanaryRequestFinished);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(reply, &QNetworkReply::error, this, &ConnectionHelper::handleCanaryRequestError);
#else
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &ConnectionHelper::handleCanaryRequestError);
#endif
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void ConnectionHelper::handleCanaryRequestError()
#else
void ConnectionHelper::handleCanaryRequestError(const QNetworkReply::NetworkError &)
#endif
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->setProperty("isError", QVariant::fromValue<bool>(true));
    reply->deleteLater();
    if (d_ptr->m_detectingNetworkConnection) {
        networkConnectivityUnavailable();
    }
}

void ConnectionHelper::handleCanaryRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply->property("isError").toBool()) {
        reply->deleteLater();
        if (d_ptr->m_detectingNetworkConnection) {
            handleNetworkEstablished();
        }
    }
}

void ConnectionHelper::emitFailureIfNeeded()
{
    // unless a successful connection was established since the call to this function
    // was queued, we should emit the error signal.
    if (d_ptr->m_detectingNetworkConnection && d_ptr->m_timeoutTimer.isActive()) {
        d_ptr->m_timeoutTimer.stop();
        QMetaObject::invokeMethod(this, "handleNetworkUnavailable", Qt::QueuedConnection);
    }
}

void ConnectionHelper::openConnectionDialog()
{
    // open Connection Selector

    if (!d_ptr->m_connectionSelectorInterface) {
        QDBusConnection connection = QDBusConnection::sessionBus();

        d_ptr->m_connectionSelectorInterface = new QDBusInterface(
                    QStringLiteral("com.jolla.lipstick.ConnectionSelector"),
                    QStringLiteral("/"),
                    QStringLiteral("com.jolla.lipstick.ConnectionSelectorIf"),
                    connection,
                    this);

        connection.connect(
                    QStringLiteral("com.jolla.lipstick.ConnectionSelector"),
                    QStringLiteral("/"),
                    QStringLiteral("com.jolla.lipstick.ConnectionSelectorIf"),
                    QStringLiteral("connectionSelectorClosed"),
                    this,
                    SLOT(handleConnectionSelectorClosed(bool)));
    }

    QList<QVariant> args;
    args.append(QStringLiteral("wifi"));
    QDBusPendingCall call = d_ptr->m_connectionSelectorInterface->asyncCallWithArgumentList(
                QStringLiteral("openConnectionNow"), args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        watcher->deleteLater();
        QDBusPendingReply<void> reply = *watcher;
        if (reply.isError()) {
            serviceErrorChanged(reply.error().message());
        } else {
            setSelectorVisible(true);
        }
    });
}

void ConnectionHelper::handleConnectionSelectorClosed(bool connectionSelected)
{
    if (!connectionSelected) {
        // canceled
        handleNetworkUnavailable();
    } else if (!d_ptr->m_netman->defaultRoute() || d_ptr->m_netman->defaultRoute()->state() != "online") {
        // perform a status request in case we are behind a captive portal
        QMetaObject::invokeMethod(this, "performRequest", Qt::QueuedConnection);
    }
    setSelectorVisible(false);
}

void ConnectionHelper::serviceErrorChanged(const QString &errorString)
{
    qWarning() << "Unable to open connection selector: " << errorString;
    handleNetworkUnavailable();
}

void ConnectionHelper::networkStateChanged(const QString &state)
{
    if (state == "online") {
        handleNetworkEstablished();
    } else if (state == "ready") {
        // potentially via captive portal, so not online.
        updateStatus(ConnectionHelper::Connected);
    } else if (state == "idle" || state == "offline") {
        handleNetworkUnavailable();
    }
}

void ConnectionHelper::handleNetworkEstablished()
{
    d_ptr->m_detectingNetworkConnection = false;
    updateStatus(ConnectionHelper::Online);
    emit networkConnectivityEstablished();
}

void ConnectionHelper::handleNetworkUnavailable()
{
    d_ptr->m_detectingNetworkConnection = false;
    updateStatus(ConnectionHelper::Offline);
    emit networkConnectivityUnavailable();
}

}
