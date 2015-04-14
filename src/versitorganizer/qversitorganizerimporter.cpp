/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtVersitOrganizer module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qversitorganizerimporter.h"
#include "qversitorganizerimporter_p.h"

#include <QtVersit/qversitdocument.h>
#include <QtVersit/qversitproperty.h>

QTVERSIT_USE_NAMESPACE

QT_BEGIN_NAMESPACE_VERSITORGANIZER

/*!
  \class QVersitOrganizerImporter
  \brief The QVersitOrganizerImporter class converts \l{QVersitDocument}{QVersitDocuments} to
  \l{QOrganizerItem}{QOrganizerItems}.

  \ingroup versit
  \inmodule QtVersit

  This class is used to convert a \l QVersitDocument (which may be produced by a
  QVersitReader) to lists of \l{QOrganizerItem}{QOrganizerItems} (which may be saved into a
  QOrganizerManager.  Unless there is an error, there is a one-to-one mapping between
  sub-documents of the input Versit document and QOrganizerItems.
 */

/*!
  \class QVersitOrganizerImporterPropertyHandler
  \brief The QVersitOrganizerImporterPropertyHandler class is an interface for specifying
  custom import behaviour for vCard properties.
  \ingroup versit-extension
  \inmodule QtVersit

  For general information on extending Qt Versit, see the document on \l{Qt Versit Plugins}.

  \sa QVersitOrganizerImporter
 */

/*!
  \fn QVersitOrganizerImporterPropertyHandler::~QVersitOrganizerImporterPropertyHandler()
  Frees any memory in use by this handler.
 */

/*!
  \fn void QVersitOrganizerImporterPropertyHandler::propertyProcessed(const QVersitDocument& document, const QVersitProperty& property, const QOrganizerItem& item, bool* alreadyProcessed, QList<QOrganizerItemDetail>* updatedDetails)
  Process \a property and provide a list of updated details by adding them to \a updatedDetails.

  This function is called on every QVersitProperty encountered during an import, after the property
  has been processed by the QVersitOrganizerImporter.  An implementation of this function can be
  made to provide support for vCard properties not supported by QVersitOrganizerImporter.

  The supplied \a document is the container for the \a property.  \a alreadyProcessed is true if the
  QVersitOrganizerImporter or another handler was successful in processing the property.  If it is
  false and the handler processes the property, it should be set to true to inform later handlers
  that the property requires no further processing.  \a item holds the state of the item before the
  property was processed by the importer.  \a updatedDetails is initially filled with a list of
  details that the importer will update, and can be modified (by removing, modifying or
  adding details to the list)
 */

/*!
  \fn void QVersitOrganizerImporterPropertyHandler::subDocumentProcessed(const QVersitDocument& topLevel, const QVersitDocument& subDocument, QOrganizerItem* item)
  Perform any final processing on the \a item generated by the \a subDocument.  \a topLevel is the
  container within which \a subDocument was found.  This can be implemented by the handler to clear
  any internal state before moving onto the next document.

  This function is called after all QVersitProperties have been handled by the
  QVersitOrganizerImporter.
*/

/*!
  \enum QVersitOrganizerImporter::Error
  This enum specifies an error that occurred during the most recent call to importDocument()
  \value NoError The most recent operation was successful
  \value InvalidDocumentError One of the documents is not an iCalendar file
  \value EmptyDocumentError One of the documents is empty
  */

/*! Constructs a new importer */
QVersitOrganizerImporter::QVersitOrganizerImporter()
    : d(new QVersitOrganizerImporterPrivate)
{
}

/*!
 * Constructs a new importer for the given \a profile.  The profile strings should be one of those
 * defined by QVersitOrganizerHandlerFactory, or a value otherwise agreed to by a \l{Qt Versit
 * Plugins}{Versit plugin}.
 *
 * The profile determines which plugins will be loaded to supplement the importer.
 */
QVersitOrganizerImporter::QVersitOrganizerImporter(const QString& profile)
    : d(new QVersitOrganizerImporterPrivate(profile))
{
}

/*! Frees the memory used by the importer */
QVersitOrganizerImporter::~QVersitOrganizerImporter()
{
    delete d;
}

/*!
 * Converts \a document into a corresponding list of QOrganizerItems.  After calling this, the
 * converted organizer items can be retrieved by calling items().
 *
 * Returns true on success.  The document should contain at least one subdocument.  In the
 * importing process, each subdocument roughly corresponds to a QOrganizerItem.  If any of the
 * subdocuments cannot be imported as organizer items (eg. they don't conform to the iCalendar
 * format), false is returned and errorMap() will return a list describing the errors that occurred.
 * The successfully imported items will still be available via items().
 *
 * \sa items(), errorMap()
 */
bool QVersitOrganizerImporter::importDocument(const QVersitDocument& document)
{
    d->mItems.clear();
    d->mErrors.clear();
    bool ok = true;
    if (document.type() != QVersitDocument::ICalendar20Type
        || document.componentType() != QStringLiteral("VCALENDAR")) {
        d->mErrors.insert(-1, QVersitOrganizerImporter::InvalidDocumentError);
        return false;
    }
    const QList<QVersitDocument> subDocuments = document.subDocuments();
    if (subDocuments.isEmpty()) {
        d->mErrors.insert(-1, QVersitOrganizerImporter::EmptyDocumentError);
        return false;
    }

    int documentIndex = 0;
    foreach (const QVersitDocument& subDocument, subDocuments) {
        QOrganizerItem item;
        QVersitOrganizerImporter::Error error;
        if (d->importDocument(document, subDocument, &item, &error)) {
            d->mItems.append(item);
        } else {
            // importDocument can return false with no error if it's a non-document component
            if (error != QVersitOrganizerImporter::NoError) {
                d->mErrors.insert(documentIndex, error);
                ok = false;
            }
        }
        documentIndex++;
    }

    return ok;
}

/*!
 * Returns the organizer items imported in the most recent call to importDocument().
 *
 * \sa importDocument()
 */
QList<QOrganizerItem> QVersitOrganizerImporter::items() const
{
    return d->mItems;
}

/*!
 * Returns the map of errors encountered in the most recent call to importDocument().
 *
 * The key is the zero based index of the sub document within the container document, or -1 for an error
 * with the container document itself.  The value is the error that occurred on that document.
 *
 * \sa importDocument()
 */
QMap<int, QVersitOrganizerImporter::Error> QVersitOrganizerImporter::errorMap() const
{
    return d->mErrors;
}

/*!
 * Sets \a handler to be the handler for processing QVersitProperties, or 0 to have no handler.
 *
 * Does not take ownership of the handler.  The client should ensure the handler remains valid for
 * the lifetime of the importer.
 *
 * Only one property handler can be set.  If another property handler was previously set, it will no
 * longer be associated with the importer.
 */
void QVersitOrganizerImporter::setPropertyHandler(QVersitOrganizerImporterPropertyHandler* handler)
{
    d->mPropertyHandler = handler;
}

QT_END_NAMESPACE_VERSITORGANIZER
