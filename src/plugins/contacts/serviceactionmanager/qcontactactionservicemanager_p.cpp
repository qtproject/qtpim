/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcontactactionservicemanager_p.h"

#include <QtContacts/qcontactaction.h>
#include <QtContacts/qcontactactiondescriptor.h>
#include <QtContacts/qcontactactionfactory.h>

#include "qservice.h"
#include "qservicemanager.h"

QTM_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QContactActionServiceManager, contactActionServiceManagerInstance)
Q_EXPORT_PLUGIN2(qtcontacts_serviceactionmanager, QContactActionServiceManager);

/*!
  \internal
  \class QContactActionServiceManager
  This class uses the service framework to discover contact actions which are
  provided by third parties.  It is an implementation detail of QContactAction.
 */

QContactActionServiceManager* QContactActionServiceManager::instance()
{
    return contactActionServiceManagerInstance();
}

QContactActionServiceManager::QContactActionServiceManager()
    : QObject(), initLock(false)
{
}

QContactActionServiceManager::~QContactActionServiceManager()
{
    // we don't use qDeleteAll() because some factories produce more than one action descriptor.
    QList<QContactActionDescriptor> keys = m_actionFactoryHash.keys();
    QSet<QContactActionFactory*> deletedFactories;
    foreach (const QContactActionDescriptor& key, keys) {
        QContactActionFactory *curr = m_actionFactoryHash.value(key);
        if (!deletedFactories.contains(curr)) {
            deletedFactories.insert(curr);
            delete curr;
        }
    }
}

void QContactActionServiceManager::init()
{
    // XXX NOTE: should already be locked PRIOR to entering this function.
    if (!initLock) {
        initLock = true;
        // fill up our hashes
        QList<QServiceInterfaceDescriptor> sids = m_serviceManager.findInterfaces(); // all services, all interfaces.
        foreach (const QServiceInterfaceDescriptor& sid, sids) {
            if (sid.interfaceName() == QContactActionFactory::InterfaceName) {
                if (static_cast<QService::Type>(sid.attribute(QServiceInterfaceDescriptor::ServiceType).toInt()) != QService::Plugin) {
                    continue; // we don't allow IPC contact action factories.
                }
                QContactActionFactory* actionFactory = qobject_cast<QContactActionFactory*>(m_serviceManager.loadInterface(sid));
                if (actionFactory) {
                    // if we cannot get the action factory from the service manager, then we don't add it to our hash.
                    QList<QContactActionDescriptor> descriptors = actionFactory->actionDescriptors();
                    foreach (const QContactActionDescriptor& ad, descriptors) {
                        m_descriptorHash.insert(ad.actionName(), ad); // multihash insert.
                        m_actionFactoryHash.insert(ad, actionFactory);
                    }
                }
            }
        }

        // and listen for signals.
        connect(&m_serviceManager, SIGNAL(serviceAdded(QString,QService::Scope)), this, SLOT(serviceAdded(QString)));
        connect(&m_serviceManager, SIGNAL(serviceRemoved(QString,QService::Scope)), this, SLOT(serviceRemoved(QString)));
    }
}

QHash<QContactActionDescriptor, QContactActionFactory*> QContactActionServiceManager::actionFactoryHash()
{
    QMutexLocker locker(&m_instanceMutex);
    init();
    return m_actionFactoryHash;
}

QMultiHash<QString, QContactActionDescriptor> QContactActionServiceManager::descriptorHash()
{
    QMutexLocker locker(&m_instanceMutex);
    init();
    return m_descriptorHash;
}

void QContactActionServiceManager::serviceAdded(const QString& serviceName)
{
    QMutexLocker locker(&m_instanceMutex);
    QList<QServiceInterfaceDescriptor> sids = m_serviceManager.findInterfaces(serviceName);
    foreach (const QServiceInterfaceDescriptor& sid, sids) {
        if (sid.interfaceName() == QContactActionFactory::InterfaceName) {
            if (static_cast<QService::Type>(sid.attribute(QServiceInterfaceDescriptor::ServiceType).toInt()) != QService::Plugin) {
                continue; // we don't allow IPC contact action factories.
            }
            QContactActionFactory* actionFactory = qobject_cast<QContactActionFactory*>(m_serviceManager.loadInterface(sid));
            if (actionFactory) {
                // if we cannot get the action factory from the service manager, then we don't add it to our hash.
                QList<QContactActionDescriptor> descriptors = actionFactory->actionDescriptors();
                foreach (const QContactActionDescriptor& ad, descriptors) {
                    m_descriptorHash.insert(ad.actionName(), ad); // multihash insert.
                    m_actionFactoryHash.insert(ad, actionFactory);
                }
            }
        }
    }
}

void QContactActionServiceManager::serviceRemoved(const QString& serviceName)
{
    QMutexLocker locker(&m_instanceMutex);
    QList<QServiceInterfaceDescriptor> sids = m_serviceManager.findInterfaces(serviceName);
    foreach (const QServiceInterfaceDescriptor& sid, sids) {
        if (sid.interfaceName() == QContactActionFactory::InterfaceName) {
            if (static_cast<QService::Type>(sid.attribute(QServiceInterfaceDescriptor::ServiceType).toInt()) != QService::Plugin) {
                continue; // we don't allow IPC contact action factories.
            }
            QList<QContactActionDescriptor> cads = m_actionFactoryHash.keys();
            foreach (const QContactActionDescriptor& cad, cads) {
                if (cad.serviceName() != serviceName)
                    continue;
                delete m_actionFactoryHash.value(cad);
                m_actionFactoryHash.remove(cad);
                m_descriptorHash.remove(cad.actionName(), cad);
            }
        }
    }
}

#include "moc_qcontactactionservicemanager_p.cpp"

QTM_END_NAMESPACE
