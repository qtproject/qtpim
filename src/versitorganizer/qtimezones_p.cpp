/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtVersitOrganizer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtimezones_p.h"
#include <qorganizer.h>
#include <QDateTime>

QTVERSITORGANIZER_BEGIN_NAMESPACE

QOrganizerManager* TimeZone::getManager()
{
    // We use the memory engine to do time zone recurrence calculations
    static QOrganizerManager* manager(new QOrganizerManager(QString::fromLatin1("memory")));
    return manager;
}

QDateTime TimeZone::convert(const QDateTime& dateTime) const
{
    Q_ASSERT(isValid());
    QOrganizerManager* manager = getManager();
    int offset = 100000; // impossible value
    QDateTime latestPhase;
    foreach(const TimeZonePhase& phase, mPhases) {
        QOrganizerEvent event;
        event.setStartDateTime(phase.startDateTime());
        event.setRecurrenceRules(QSet<QOrganizerRecurrenceRule>() << phase.recurrenceRule());
        event.setRecurrenceDates(phase.recurrenceDates());
        QList<QOrganizerItem> occurrences =
            manager->itemOccurrences(event, phase.startDateTime(), dateTime, 500);
        if (!occurrences.isEmpty()) {
            QDateTime phaseStart(static_cast<const QOrganizerEventOccurrence &>(occurrences.last()).startDateTime());
            if (phaseStart > latestPhase) {
                latestPhase = phaseStart;
                offset = phase.utcOffset();
            }
        }
        else {
            offset = phase.utcOffset();
        }
    }
    QDateTime retn(dateTime);
    retn.setTimeSpec(Qt::UTC);
    if (offset >= -86400 && offset <= 86400) // offset must be within -24hours to +24hours
        return retn.addSecs(-offset);
    else
        return retn;
}

QDateTime TimeZones::convert(const QDateTime& dateTime, const QString& tzid) const
{
    if (!mTimeZones.contains(tzid))
        return QDateTime();
    TimeZone tz = mTimeZones.value(tzid);
    if (!tz.isValid())
        return QDateTime();
    return tz.convert(dateTime);
}

QTVERSITORGANIZER_END_NAMESPACE
