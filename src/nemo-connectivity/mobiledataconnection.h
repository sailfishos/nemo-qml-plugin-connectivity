/* Copyright (C) 2017 Jolla Ltd.
 * Contact: Raine Makelainen <raine.makelainen@jolla.com>
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

#ifndef NEMO_MOBILEDATACONNECTION_H
#define NEMO_MOBILEDATACONNECTION_H

#include <nemo-connectivity/global.h>

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(CONNECTIVITY)

namespace Nemo {

class MobileDataConnectionPrivate;

class NEMO_CONNECTIVITY_EXPORT MobileDataConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    Q_PROPERTY(bool useDefaultModem READ useDefaultModem WRITE setUseDefaultModem NOTIFY useDefaultModemChanged)

    Q_PROPERTY(QString connectionName READ connectionName NOTIFY connectionNameChanged)
    Q_PROPERTY(QString modemPath READ modemPath WRITE setModemPath NOTIFY modemPathChanged)
    Q_PROPERTY(QString defaultDataSim READ defaultDataSim WRITE setDefaultDataSim NOTIFY defaultDataSimChanged)
    Q_PROPERTY(int presentSimCount READ presentSimCount NOTIFY presentSimCountChanged)

    Q_PROPERTY(int slotCount READ slotCount NOTIFY slotCountChanged)
    Q_PROPERTY(int slotIndex READ slotIndex NOTIFY slotIndexChanged)

    Q_PROPERTY(QString subscriberIdentity READ subscriberIdentity NOTIFY subscriberIdentityChanged)
    Q_PROPERTY(QString serviceProviderName READ serviceProviderName NOTIFY serviceProviderNameChanged)

    Q_PROPERTY(QString identifier READ identifier NOTIFY identifierChanged)

    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    Q_PROPERTY(bool offlineMode READ offlineMode NOTIFY offlineModeChanged)

    Q_PROPERTY(bool roamingAllowed READ roamingAllowed NOTIFY roamingAllowedChanged)
    Q_PROPERTY(bool roaming READ roaming NOTIFY roamingChanged)

    Q_PROPERTY(bool saved READ saved NOTIFY savedChanged)

public:
    MobileDataConnection();
    ~MobileDataConnection();

    enum Status {
        Unknown = -1,
        Disconnected,
        Connecting,
        Limited,
        Online
    };
    Q_ENUM(Status)

    bool isValid() const;

    bool autoConnect() const;
    void setAutoConnect(bool autoConnect);

    bool connected() const;

    Status status() const;

    bool useDefaultModem() const;
    void setUseDefaultModem(bool useDefaultModem);

    QString connectionName() const;

    QString modemPath() const;
    void setModemPath(const QString &modemPath);

    QString defaultDataSim() const;
    void setDefaultDataSim(const QString &subscriberIdentity);

    int presentSimCount() const;

    int slotCount() const;
    int slotIndex() const;

    QString subscriberIdentity() const;
    QString serviceProviderName() const;

    QString identifier() const;

    QString error() const;

    bool offlineMode() const;

    bool roamingAllowed() const;
    bool roaming() const;

    bool saved() const;

    Q_INVOKABLE void connect();
    Q_INVOKABLE void disconnect();

Q_SIGNALS:
    void validChanged();
    void autoConnectChanged();
    void connectedChanged();

    void statusChanged();

    void useDefaultModemChanged();
    void connectionNameChanged();
    void modemPathChanged();
    void defaultDataSimChanged();

    void presentSimCountChanged();

    void slotCountChanged();
    void slotIndexChanged();

    void subscriberIdentityChanged();
    void serviceProviderNameChanged();

    void identifierChanged();

    void errorChanged();
    void offlineModeChanged();

    void roamingAllowedChanged();
    void roamingChanged();

    void savedChanged();

    void reportError(const QString &errorString);

private:
    MobileDataConnectionPrivate *d_ptr;
    Q_DISABLE_COPY(MobileDataConnection)
    Q_DECLARE_PRIVATE(MobileDataConnection)
};

}

#endif
