/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtContacts module of the Qt Toolkit.
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

#ifndef QDECLARATIVECONTACTTYPE_H
#define QDECLARATIVECONTACTTYPE_H

#include <QtContacts/qcontacttype.h>

#include "qdeclarativecontactdetail_p.h"

QTCONTACTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class QDeclarativeContactType : public QDeclarativeContactDetail
{
    Q_OBJECT
    Q_PROPERTY(ContactType type READ type WRITE setType NOTIFY valueChanged)
    Q_ENUMS(FieldType)
    Q_ENUMS(ContactType)
    Q_CLASSINFO("DefaultProperty", "type")
public:
    enum FieldType {
        TypeField = QContactType::FieldType
    };

    enum ContactType {
        Unspecified = 0,
        Contact = QContactType::TypeContact,
        Group = QContactType::TypeGroup,
        Facet = QContactType::TypeFacet
    };


    DetailType detailType() const
    {
        return QDeclarativeContactDetail::Type;
    }

    QDeclarativeContactType(QObject* parent = nullptr)
        :QDeclarativeContactDetail(parent)
    {
        setDetail(QContactType());
        connect(this, SIGNAL(valueChanged()), SIGNAL(detailChanged()));
    }

    void setType(ContactType newType)
    {
        if (!readOnly() && newType!=type())  {
            switch (newType) {
            case Contact:
            case Group:
            case Facet:
                detail().setValue(QContactType::FieldType, newType);
                break;
            default:
                detail().setValue(QContactType::FieldType, Unspecified);
            }
        }
    }

    ContactType type() const
    {
        return static_cast<ContactType>(detail().value(QContactType::FieldType).toInt());
    }
signals:
    void valueChanged();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeContactType)

#endif // QDECLARATIVECONTACTTYPE_H
