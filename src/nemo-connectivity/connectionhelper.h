/* Copyright (C) 2013-2017 Jolla Ltd.
 * Contact: Chris Adams <chris.adams@jolla.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Jolla Ltd. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef NEMO_CONNECTION_HELPER_H
#define NEMO_CONNECTION_HELPER_H

#include <QObject>

#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <nemo-connectivity/global.h>

#include <connman-qt5/networkmanager.h>
#include <connman-qt5/networktechnology.h>
#include <connman-qt5/networkservice.h>

#include <QTimer>

QT_BEGIN_NAMESPACE
class QDBusInterface;
QT_END_NAMESPACE

namespace Nemo {

class NEMO_CONNECTIVITY_EXPORT ConnectionHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)

public:
    ConnectionHelper(QObject *parent = 0);
    ~ConnectionHelper();

    Q_INVOKABLE void attemptToConnectNetwork();
    Q_INVOKABLE void requestNetwork();

    bool online() const;

Q_SIGNALS:
    void networkConnectivityEstablished();
    void networkConnectivityUnavailable();
    void onlineChanged();

private Q_SLOTS:
    void performRequest();
    void handleCanaryRequestError(const QNetworkReply::NetworkError &error);
    void handleCanaryRequestFinished();
    void emitFailureIfNeeded(); // due to timeout.

    void handleConnectionSelectorClosed(bool);
    void connmanAvailableChanged(bool);
    void serviceErrorChanged(const QString &);
    void networkStateChanged(const QString &);
    void openConnectionDialog();

private:
    void handleNetworkEstablished();
    void handleNetworkUnavailable();
    void _attemptToConnectNetwork(bool explicitAttempt);

private:
    QTimer m_timeoutTimer;
    QNetworkAccessManager *m_networkAccessManager;
    bool m_networkConfigReady;
    bool m_delayedAttemptToConnect;
    bool m_detectingNetworkConnection;
    bool m_connmanIsAvailable;
    bool m_online;

    NetworkManager *m_netman;

    QDBusInterface *m_connectionSelectorInterface;
};

}

#endif
