// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ContainerRegistry.h"

#include "ContainerDetector.h"
#include "ContainerInfo.h"
#include "DistroboxDetector.h"
#include "KapsuleDetector.h"
#include "ToolboxDetector.h"

#include <KLocalizedString>
#include <QSysInfo>
#include <QTimer>
#include <QVariantMap>

namespace Kwi
{
ContainerRegistry::ContainerRegistry(QObject *parent)
    : QObject{parent}
    , m_detectors{new ToolboxDetector{this}, new DistroboxDetector{this}, new KapsuleDetector{this}}
{
    for (auto *detector : m_detectors) {
        connect(detector, &ContainerDetector::listContainersFinished, this, [this, detector](const QStringList &names) {
            m_names.insert(detector->typeId(), names);

            if (m_pendingRefreshes > 0) {
                --m_pendingRefreshes;
            }

            if (m_pendingRefreshes == 0) {
                rebuildEntries();
                Q_EMIT containersChanged();
            }
        });
    }

    rebuildEntries();

    QTimer::singleShot(0, this, &ContainerRegistry::refresh);
}

QVariantList ContainerRegistry::menuEntries() const
{
    return m_menuEntries;
}

QStringList ContainerRegistry::entryCommandForId(const QString &id) const
{
    const auto info{splitContainerId(id)};

    if (!info) {
        return {};
    }

    for (const auto *detector : m_detectors) {
        if (detector->typeId() == info->type) {
            return detector->entryCommand(info->name);
        }
    }

    return {};
}

void ContainerRegistry::refresh()
{
    if (m_pendingRefreshes > 0) {
        return;
    }

    m_pendingRefreshes = int(m_detectors.size());

    for (auto *detector : m_detectors) {
        detector->startListContainers();
    }
}

void ContainerRegistry::rebuildEntries()
{
    QVariantList entries;

    QString hostName{QSysInfo::machineHostName()};

    if (hostName.isEmpty()) {
        hostName = i18nc("@item:inmenu container host", "This Computer");
    }

    entries.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("header")}, {QStringLiteral("label"), i18nc("@title:menu container section", "Host")}});
    entries.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("host")},
                               {QStringLiteral("label"), hostName},
                               {QStringLiteral("iconName"), QStringLiteral("computer")}});

    for (const auto *detector : m_detectors) {
        const QStringList names{m_names.value(detector->typeId())};

        if (names.isEmpty()) {
            continue;
        }

        entries.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("header")}, {QStringLiteral("label"), detector->displayName()}});

        for (const QString &name : names) {
            entries.append(QVariantMap{{QStringLiteral("kind"), QStringLiteral("container")},
                                       {QStringLiteral("label"), name},
                                       {QStringLiteral("type"), detector->typeId()},
                                       {QStringLiteral("name"), name},
                                       {QStringLiteral("iconName"), detector->iconName()}});
        }
    }

    m_menuEntries = entries;
}
}
