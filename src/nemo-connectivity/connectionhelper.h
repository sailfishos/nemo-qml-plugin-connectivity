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
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#endif

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <nemo-connectivity/global.h>

QT_BEGIN_NAMESPACE
class QDBusInterface;
QT_END_NAMESPACE

class ConnectionHelperPrivate;

namespace Nemo {

class NEMO_CONNECTIVITY_EXPORT ConnectionHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(bool selectorVisible READ selectorVisible NOTIFY selectorVisibleChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

public:
    ConnectionHelper(QObject *parent = 0);
    ~ConnectionHelper();

    Q_INVOKABLE void attemptToConnectNetwork();
    Q_INVOKABLE void requestNetwork();

    bool selectorVisible() const;
    bool online() const;

    enum Status {
        Offline = 0,
        Connecting,
        Connected,
        Online
    };
    Q_ENUM(Status)
    Status status() const;

Q_SIGNALS:
    void networkConnectivityEstablished();
    void networkConnectivityUnavailable();
    void onlineChanged();
    void selectorVisibleChanged();
    void statusChanged();

private Q_SLOTS:
    void performRequest();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void handleCanaryRequestError();
#else
    void handleCanaryRequestError(const QNetworkReply::NetworkError &error);
#endif
    void handleCanaryRequestFinished();
    void emitFailureIfNeeded(); // due to timeout.

    void handleNetworkEstablished();
    void handleNetworkUnavailable();
    void handleConnectionSelectorClosed(bool);
    void connmanAvailableChanged(bool);
    void serviceErrorChanged(const QString &);
    void networkStateChanged(const QString &);
    void openConnectionDialog();

    void getConnmanManagerProperties(const QVariantMap &props);

private:
    void updateStatus(Status status);
    void determineDefaultNetworkStatusCheckUrl();
    void _attemptToConnectNetwork(bool explicitAttempt);
    void setSelectorVisible(bool selectorVisible);

private:
    ConnectionHelperPrivate *d_ptr;
};

}

#endif
