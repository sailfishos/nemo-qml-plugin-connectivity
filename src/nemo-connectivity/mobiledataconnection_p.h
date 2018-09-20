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

#ifndef NEMO_MOBILEDATACONNECTION_P_H
#define NEMO_MOBILEDATACONNECTION_P_H

#include <QSharedPointer>

#include <qofonoextmodemmanager.h>
#include <qofonoconnectionmanager.h>
#include <qofonosimmanager.h>

#include <networkmanager.h>
#include <networkservice.h>

#include <qofonoconnectioncontext.h>
#include <qofononetworkregistration.h>

namespace Nemo {

class MobileDataConnection;

class MobileDataConnectionPrivate
{
public:
    MobileDataConnectionPrivate(Nemo::MobileDataConnection *q);
    ~MobileDataConnectionPrivate();

    bool isValid() const;
    void updateValid();
    bool isSimManagerValid() const;

    void updateStatus();

    QString subscriberIdentity() const;
    void updateSubscriberIdentity();
    void updateServiceProviderName();

    QString servicePathForContext();

    bool isDataContextReady(const QString &modemPath) const;

    void createDataContext(const QString &modemPath);
    void updateDataContext();
    bool hasDataContext();

    void requestConnect();

    bool valid;
    bool simManagerValid;

    MobileDataConnection::Status status;
    bool connectingService;

    bool useDefaultModem;

    QString inetContextPath;
    QString connectionName;
    QString modemPath;
    QString serviceProviderName;

    MobileDataConnection *q;

    QSharedPointer<QOfonoExtModemManager> modemManager;

    QOfonoSimManager simManager;

    NetworkManager networkManager;
    NetworkService *networkService;
    QOfonoNetworkRegistration networkRegistration;

    QSharedPointer<QOfonoConnectionManager> connectionManager;
    QOfonoConnectionContext *connectionContext;
};

}
#endif
