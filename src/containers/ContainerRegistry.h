// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ContainerInfo.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQmlIntegration/qqmlintegration.h>

namespace Kwi
{
class ContainerDetector;

/**
 * The set of containers new sessions can be started in.
 *
 * Owns one detector per container tool, caches their listings, and
 * builds the flattened entry list the New Tab menu renders: a Host
 * entry first, then one section per tool that has containers. The
 * entry command for an identifier does not depend on the listing.
 */
// Not final: Qt's QML type registration derives a wrapper from
// QML_ELEMENT types (QQmlElement<T>), which a final class forbids.
class ContainerRegistry : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("The container registry is provided by the application.")
    Q_PROPERTY(QVariantList menuEntries READ menuEntries NOTIFY containersChanged)

public:
    explicit ContainerRegistry(QObject *parent = nullptr);

    /**
     * Flattened menu model. Each entry is a map with a "kind" of
     * "header", "host" or "container", a "label", and for containers
     * also "type", "name" and "iconName".
     */
    [[nodiscard]] QVariantList menuEntries() const;

    /**
     * The command that enters the container named by an identifier
     * string ("<type>:<name>"), or an empty list if the identifier is
     * malformed or its type is unknown.
     */
    [[nodiscard]] QStringList entryCommandForId(const QString &id) const;

    /** Re-list all containers in the background. */
    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void containersChanged();

private:
    void rebuildEntries();

    QList<ContainerDetector *> m_detectors{};
    QHash<QString, QStringList> m_names{};
    QVariantList m_menuEntries{};
    int m_pendingRefreshes{0};
};
}
