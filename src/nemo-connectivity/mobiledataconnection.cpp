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

#include "mobiledataconnection.h"
#include "mobiledataconnection_p.h"

Q_LOGGING_CATEGORY(CONNECTIVITY, "qt.nemo.connectivity")

namespace Nemo {

MobileDataConnectionPrivate::MobileDataConnectionPrivate(MobileDataConnection *q)
    : valid(false)
    , simManagerValid(false)
    , autoConnect(false)
    , autoConnectPending(false)
    , status(MobileDataConnection::Unknown)
    , connectingService(false)
    , useDefaultModem(false)
    , q(q)
    , modemManager(QOfonoExtModemManager::instance())
    , simManager(q)
    , networkManager(q)
    , networkService(new NetworkService(q))
    , networkRegistration(q)
    , connectionManager(nullptr)
    , connectionContext(nullptr)
{
}

MobileDataConnectionPrivate::~MobileDataConnectionPrivate()
{
    modemManager.reset();

    delete networkService;
    networkService = nullptr;

    connectionManager.reset();
    delete connectionContext;
    connectionContext = nullptr;
}

bool MobileDataConnectionPrivate::isValid() const
{
    return networkService && networkService->isValid() && !networkService->path().isEmpty()
            && connectionManager && connectionManager->isValid()
            && connectionContext && connectionContext->isValid()
            && networkService->available();
}

void MobileDataConnectionPrivate::updateValid()
{
    bool v = isValid();
    qCDebug(CONNECTIVITY, "Update valid old: %d new: %d modem: %s connecting: %d %s available: %d %s",
            valid, v, qPrintable(q->modemPath()), connectingService, qPrintable(networkService->path()),
            networkService->available(), qPrintable(q->objectName()));
    if (valid != v) {
        valid = v;
        emit q->validChanged();
    }
}

bool MobileDataConnectionPrivate::isSimManagerValid() const
{
    return simManager.isValid() && simManager.present();
}

void MobileDataConnectionPrivate::updateStatus()
{
    MobileDataConnection::Status oldStatus = status;
    QString state = networkService->state();

    bool connecting = (state == QLatin1String("association") || state == QLatin1String("configuration"));
    if (q->connected()) {
        connectingService = false;
    }

    if (state == QLatin1String("online")) {
        status = MobileDataConnection::Online;
    } else if (state == QLatin1String("ready")) {
        status = MobileDataConnection::Limited;
    } else if (connectingService || connecting) {
        status = MobileDataConnection::Connecting;
    } else {
        status = MobileDataConnection::Disconnected;
    }

    if (oldStatus != status) {
        emit q->statusChanged();
    }

    qCDebug(CONNECTIVITY, "Update status: %d old: %d state: %s connecting service: %d %s", status, oldStatus,
            qPrintable(state), connectingService, qPrintable(q->objectName()));
}

void MobileDataConnectionPrivate::updateNetworkServicePath()
{
    bool simMgrValid = isSimManagerValid();
    if (simMgrValid != simManagerValid) {
        simManagerValid = simMgrValid;
        updateSubscriberIdentity();
        updateServiceProviderName();
        networkService->setPath(servicePathForContext());
    }
}

void MobileDataConnectionPrivate::updateSubscriberIdentity()
{
    QString newSubscriberIdentity = isSimManagerValid() ? simManager.subscriberIdentity() : QString();
    if (subscriberIdentity != newSubscriberIdentity) {
        subscriberIdentity = newSubscriberIdentity;
        emit q->subscriberIdentityChanged();
    }
}

void MobileDataConnectionPrivate::updateServiceProviderName()
{
    QString newName = isSimManagerValid() ? simManager.serviceProviderName() : QString();
    if (serviceProviderName != newName) {
        serviceProviderName = newName;
        emit q->serviceProviderNameChanged();
    }
}

QString MobileDataConnectionPrivate::servicePathForContext()
{
    if (inetContextPath.isEmpty()) {
        return QString();
    }

    QString imsi = subscriberIdentity;
    if (imsi.isEmpty()) {
        return QString();
    }

    QStringList cellularServices = networkManager.servicesList(QLatin1String("cellular"));
    if (cellularServices.isEmpty()) {
        return QString();
    }

    QString context = inetContextPath.section('/', -1);
    QString servicePath = "/net/connman/service/cellular_" + imsi + "_" + context;

    if (cellularServices.contains(servicePath)) {
        qCDebug(CONNECTIVITY, "Service path for context: %s %s",
                qPrintable(servicePath), qPrintable(q->objectName()));
        return servicePath;
    }

    return QString();
}

bool MobileDataConnectionPrivate::isDataContextReady(const QString &modemPath) const
{
    return connectionManager && connectionManager->powered() &&
            !connectionManager->contexts().isEmpty() && !modemPath.isEmpty();
}

void MobileDataConnectionPrivate::createDataContext(const QString &modemPath)
{
    if (hasDataContext() || modemPath.isEmpty()) {
        return;
    }

    connectionManager = QOfonoConnectionManager::instance(modemPath);
    connectionManager->setFilter(QLatin1String("internet"));

    QObject::connect(connectionManager.data(), &QOfonoConnectionManager::roamingAllowedChanged,
                     q, &MobileDataConnection::roamingAllowedChanged);
    QObject::connect(connectionManager.data(), &QOfonoConnectionManager::validChanged, q, [=]() {
        qCDebug(CONNECTIVITY, "QOfonoConnectionManager::validChanged");
        updateValid();
    });

    QObject::connect(connectionManager.data(), &QOfonoConnectionManager::contextsChanged, q, [=]() {
        qCDebug(CONNECTIVITY, "QOfonoConnectionManager::contextsChanged context path: %s %s %s",
                qPrintable(connectionManager->contexts().join(",")), qPrintable(q->modemPath()),
                qPrintable(q->objectName()));
        updateDataContext();
    });

    QObject::connect(connectionManager.data(), &QOfonoConnectionManager::poweredChanged, q, [=]() {
        updateDataContext();
    });

    connectionContext = new QOfonoConnectionContext(q);
    QObject::connect(connectionContext, &QOfonoConnectionContext::nameChanged, q, [=](const QString &name) {
        if (connectionName != name) {
            connectionName = name;
            emit q->connectionNameChanged();
        }
    });

    QObject::connect(connectionContext, &QOfonoConnectionContext::contextPathChanged, q, [=](const QString &contextPath) {
        if (contextPath.isEmpty()) {
            networkService->setPath(QString());
        } else {
            networkService->setPath(servicePathForContext());
        }
    });
    QObject::connect(connectionContext, &QOfonoConnectionContext::validChanged, q, [=]() {
        qCDebug(CONNECTIVITY, "QOfonoConnectionContext::validChanged");
        updateValid();
    });

    QObject::connect(connectionContext, &QOfonoConnectionContext::reportError, q, &MobileDataConnection::reportError);
}

void MobileDataConnectionPrivate::updateDataContext()
{
    // Use modemPath of the SimManager as that contains always the current modem.
    QString modemPath = simManager.modemPath();
    createDataContext(modemPath);
    if (modemPath.isEmpty()) {
        qCDebug(CONNECTIVITY, "####### Clear data context has data context: %d auto connect: %d %s %s connecting: %d %s\n\n",
                hasDataContext(), networkService->autoConnect(), qPrintable(q->modemPath()),
                qPrintable(networkService->path()), connectingService, qPrintable(q->objectName()));
        inetContextPath = "";
        connectingService = false;

        if (hasDataContext()) {
            connectionContext->setContextPath(QLatin1String(""));
        }
    } else if (isDataContextReady(modemPath)) {
        QStringList contexts = connectionManager->contexts();
        qCDebug(CONNECTIVITY, "####### Set data context: %s m: %s %s", qPrintable(contexts.join(",")),
                qPrintable(q->modemPath()), qPrintable(q->objectName()));
        if (contexts.count() > 0) {
            inetContextPath = contexts.at(0);
        } else {
            inetContextPath = "";
        }
        connectionContext->setContextPath(inetContextPath);
    } else if (hasDataContext() && !connectionManager->powered()) {
        connectionManager->setPowered(true);
    }

}

bool MobileDataConnectionPrivate::hasDataContext()
{
    return connectionManager && connectionContext;
}

void MobileDataConnectionPrivate::requestConnect()
{
    if (connectingService && valid && !q->modemPath().isEmpty() && networkService->available()) {
        qCDebug(CONNECTIVITY,
                "\n\n\n\n\n================================= MobileDataConnection requestConnect: %s %s\n\n\n\n\n",
                qPrintable(q->modemPath()), qPrintable(q->objectName()));
        networkService->requestConnect();
        updateStatus();
    }
}

MobileDataConnection::MobileDataConnection()
    : d_ptr(new MobileDataConnectionPrivate(this))
{
    QObject::connect(&d_ptr->simManager, &QOfonoSimManager::modemPathChanged,
            this, [=](QString modemPath) {
        d_ptr->networkRegistration.setModemPath(modemPath);

        if (d_ptr->connectionManager && modemPath != d_ptr->connectionManager->modemPath()) {
            QObject::disconnect(d_ptr->connectionManager.data(), 0, this, 0);
            d_ptr->connectionManager.reset();
            delete d_ptr->connectionContext;
            d_ptr->connectionContext = nullptr;
            d_ptr->updateDataContext();
        }

        emit modemPathChanged();
        emit slotIndexChanged();
        qCDebug(CONNECTIVITY, "QOfonoSimManager::modemPathChanged %s index: %d", qPrintable(modemPath), slotIndex());
    });

    QObject::connect(&d_ptr->simManager, &QOfonoSimManager::validChanged, this, [=]() {
        d_ptr->updateNetworkServicePath();
    });
    QObject::connect(&d_ptr->simManager, &QOfonoSimManager::presenceChanged, this, [=]() {
        d_ptr->updateNetworkServicePath();
    });
    QObject::connect(&d_ptr->simManager, &QOfonoSimManager::subscriberIdentityChanged, this, [=]() {
        d_ptr->updateSubscriberIdentity();
    });
    QObject::connect(&d_ptr->simManager, &QOfonoSimManager::serviceProviderNameChanged, this, [=]() {
        d_ptr->updateServiceProviderName();
    });

    QObject::connect(&d_ptr->networkManager, &NetworkManager::availabilityChanged, this, [=]() {
        d_ptr->networkService->setPath(d_ptr->servicePathForContext());
    });

    QObject::connect(&d_ptr->networkManager, &NetworkManager::cellularServicesChanged, this, [=]() {
        d_ptr->networkService->setPath(d_ptr->servicePathForContext());
    });

    QObject::connect(&d_ptr->networkManager, &NetworkManager::offlineModeChanged,
            this, &MobileDataConnection::offlineModeChanged);


    QObject::connect(d_ptr->networkService, &NetworkService::errorChanged, this, [=](const QString &error) {
        if (!error.isEmpty()) {
            d_ptr->connectingService = false;
        }
        emit errorChanged();
    });
    QObject::connect(d_ptr->networkService, &NetworkService::stateChanged, this, [=]() {
        // This and available should be clearly visible in the logs.
        qCDebug(CONNECTIVITY, "####################### MobileDataConnection::stateChanged state: %s %s available: %d %s",
                qPrintable(d_ptr->networkService->state()), qPrintable(modemPath()),
                d_ptr->networkService->available(), qPrintable(objectName()));
        d_ptr->updateStatus();
        emit stateChanged();
    });

    QObject::connect(d_ptr->networkService, &NetworkService::validChanged, this, [=]() {
        qCDebug(CONNECTIVITY, "NetworkService::validChanged mobile data valid old: %d new %d", d_ptr->valid, d_ptr->isValid());
        d_ptr->updateValid();
        if (d_ptr->autoConnectPending) {
            d_ptr->networkService->setAutoConnect(d_ptr->autoConnect);
            d_ptr->autoConnectPending = false;
        }
    });

    QObject::connect(d_ptr->networkService, &NetworkService::availableChanged, this, [=]() {
        qCDebug(CONNECTIVITY, "####################### MobileDataConnection::availableChanged state: %s %s available: %d %s",
                qPrintable(d_ptr->networkService->state()), qPrintable(modemPath()),
                d_ptr->networkService->available(), qPrintable(objectName()));
        d_ptr->updateValid();
        d_ptr->updateStatus();
        emit stateChanged();
    });

    QObject::connect(d_ptr->networkService, &NetworkService::connectedChanged, this, &MobileDataConnection::connectedChanged);
    QObject::connect(d_ptr->networkService, &NetworkService::autoConnectChanged, this, [=]() {
        qCDebug(CONNECTIVITY, "NetworkService::autoConnectChanged a: %d c: %d v: %d modem: %s s: %s available: %d",
                d_ptr->networkService->autoConnect(), d_ptr->connectingService,
                d_ptr->valid, qPrintable(d_ptr->modemManager->defaultDataModem()),
                qPrintable(state()), d_ptr->networkService->available());
        if (!d_ptr->autoConnectPending) {
            emit autoConnectChanged();
        }
    });

    QObject::connect(d_ptr->networkService, &NetworkService::pathChanged, this, [=]() {
        qCDebug(CONNECTIVITY, "NetworkService::pathChanged %s modem: %s",
                qPrintable(d_ptr->networkService->path()), qPrintable(d_ptr->modemManager->defaultDataModem()));
        d_ptr->updateValid();
        emit identifierChanged();
    });

    QObject::connect(d_ptr->networkService, &NetworkService::savedChanged, this, &MobileDataConnection::savedChanged);

    QObject::connect(&d_ptr->networkRegistration, &QOfonoNetworkRegistration::statusChanged,
            this, &MobileDataConnection::roamingChanged);

    QObject::connect(d_ptr->modemManager.data(), &QOfonoExtModemManager::defaultDataSimChanged,
                     this, [=]() {
        qCDebug(CONNECTIVITY, "QOfonoExtModemManager::defaultDataSimChanged: %s %s %s",
                qPrintable(d_ptr->servicePathForContext()), qPrintable(modemPath()), qPrintable(objectName()));
        d_ptr->networkService->setPath(d_ptr->servicePathForContext());
        emit defaultDataSimChanged();
    });
    QObject::connect(d_ptr->modemManager.data(), &QOfonoExtModemManager::presentSimCountChanged,
            this, &MobileDataConnection::presentSimCountChanged);
    QObject::connect(d_ptr->modemManager.data(), &QOfonoExtModemManager::availableModemsChanged,
            this, &MobileDataConnection::slotCountChanged);
    QObject::connect(d_ptr->modemManager.data(), &QOfonoExtModemManager::defaultDataModemChanged,
                     this, [=](QString modemPath) {
        qCDebug(CONNECTIVITY, "QOfonoExtModemManager::defaultDataModemChanged: %s use default: %d",
                qPrintable(modemPath), d_ptr->useDefaultModem);
        if (d_ptr->useDefaultModem) {
            d_ptr->simManager.setModemPath(modemPath);
            d_ptr->updateDataContext();
        }
    });
}

MobileDataConnection::~MobileDataConnection()
{
    delete d_ptr;
    d_ptr = nullptr;
}

bool MobileDataConnection::isValid() const
{
    Q_D(const MobileDataConnection);
    return d->valid;
}

bool MobileDataConnection::autoConnect() const
{
    Q_D(const MobileDataConnection);
    if (d->autoConnectPending)
        return d->autoConnect;
    return d->networkService->autoConnect();
}

void MobileDataConnection::setAutoConnect(bool autoConnect)
{
    Q_D(MobileDataConnection);
    if (d->networkService->isValid()) {
        d->networkService->setAutoConnect(autoConnect);
    } else {
        d->autoConnect = autoConnect;
        d->autoConnectPending = true;
    }
}

bool MobileDataConnection::connected() const
{
    Q_D(const MobileDataConnection);
    return d->networkService->connected();
}

MobileDataConnection::Status MobileDataConnection::status() const
{
    Q_D(const MobileDataConnection);
    return d->status;
}

bool MobileDataConnection::useDefaultModem() const
{
    Q_D(const MobileDataConnection);
    return d->useDefaultModem;
}

void MobileDataConnection::setUseDefaultModem(bool useDefaultModem)
{
    Q_D(MobileDataConnection);
    if (useDefaultModem != d->useDefaultModem) {
        d->useDefaultModem = useDefaultModem;
        QString modemPath;
        if (d->useDefaultModem) {
            modemPath = d->modemManager->defaultDataModem();
        } else {
            modemPath = d->modemPath;
        }
        d->simManager.setModemPath(modemPath);
        d->updateDataContext();
        emit useDefaultModemChanged();
    }
}

QString MobileDataConnection::connectionName() const
{
    Q_D(const MobileDataConnection);
    return d->connectionName;
}

QString MobileDataConnection::modemPath() const
{
    Q_D(const MobileDataConnection);
    return d->simManager.modemPath();
}

void MobileDataConnection::setModemPath(const QString &modemPath)
{
    Q_D(MobileDataConnection);
    d->modemPath = modemPath;

    if (!d->useDefaultModem) {
        d->simManager.setModemPath(modemPath);
    }

    d->updateDataContext();
}

QString MobileDataConnection::defaultDataSim() const
{
    Q_D(const MobileDataConnection);
    return d->modemManager->defaultDataSim();
}

void MobileDataConnection::setDefaultDataSim(const QString &defaultDataSim)
{
    Q_D(MobileDataConnection);
    d->modemManager->setDefaultDataSim(defaultDataSim);
}

int MobileDataConnection::presentSimCount() const
{
    Q_D(const MobileDataConnection);
    return d->modemManager->presentSimCount();
}

int MobileDataConnection::slotCount() const
{
    Q_D(const MobileDataConnection);
    return d->modemManager->availableModems().count();
}

int MobileDataConnection::slotIndex() const
{
    Q_D(const MobileDataConnection);
    return d->modemManager->availableModems().indexOf(modemPath());
}

QString MobileDataConnection::subscriberIdentity() const
{
    Q_D(const MobileDataConnection);
    return d->subscriberIdentity;
}

QString MobileDataConnection::serviceProviderName() const
{
    Q_D(const MobileDataConnection);
    return d->serviceProviderName;
}

QString MobileDataConnection::state() const
{
    Q_D(const MobileDataConnection);
    return d->networkService->state();
}

QString MobileDataConnection::identifier() const
{
    Q_D(const MobileDataConnection);
    return d->networkService->path();
}

QString MobileDataConnection::error() const
{
    Q_D(const MobileDataConnection);
    return d->networkService->error();
}

bool MobileDataConnection::offlineMode() const
{
    Q_D(const MobileDataConnection);
    return d->networkManager.offlineMode();
}

bool MobileDataConnection::roamingAllowed() const
{
    Q_D(const MobileDataConnection);
    return d->connectionManager && d->connectionManager->roamingAllowed();
}

bool MobileDataConnection::roaming() const
{
    Q_D(const MobileDataConnection);
    return d->networkRegistration.status() == QLatin1String("roaming");
}

bool MobileDataConnection::saved() const
{
    Q_D(const MobileDataConnection);
    return d->networkService->saved();
}

void MobileDataConnection::connect()
{
    Q_D(MobileDataConnection);
    qCDebug(CONNECTIVITY, "Connect: %d valid: %d", autoConnect(), isValid());
    d->connectingService = true;
    d->requestConnect();
    d->updateStatus();
}

void MobileDataConnection::disconnect()
{
    Q_D(MobileDataConnection);
    d->networkService->requestDisconnect();
    d->connectingService = false;
    d->updateStatus();
}

}
