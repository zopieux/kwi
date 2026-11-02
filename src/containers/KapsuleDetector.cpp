// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "KapsuleDetector.h"

#include <KLocalizedString>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

namespace Kwi
{
KapsuleDetector::KapsuleDetector(QObject *parent)
    : ShellContainerDetector{parent}
{
}

QString KapsuleDetector::typeId() const
{
    return QStringLiteral("kapsule");
}

QString KapsuleDetector::displayName() const
{
    return i18nc("@title:menu container type", "Kapsule");
}

QString KapsuleDetector::iconName() const
{
    return QStringLiteral("utilities-terminal");
}

QStringList KapsuleDetector::entryCommand(const QString &containerName) const
{
    return {QStringLiteral("kapsule"), QStringLiteral("enter"), containerName};
}

QStringList KapsuleDetector::listCommand() const
{
    return {QStringLiteral("kapsule"), QStringLiteral("list")};
}

QStringList KapsuleDetector::parseNames(const QString &output) const
{
    // "kapsule list" prints a table headed "NAME STATUS IMAGE MODE
    // CREATED" (or "No containers found."), styling the header and the
    // status column with ANSI colours. Strip the styling, then take
    // the first column of each row below the header.
    static const QRegularExpression ansi{QStringLiteral("\x1B\\[[0-9;?]*[A-Za-z]")};
    static const QRegularExpression whitespace{QStringLiteral("\\s+")};

    QStringList names;
    const QStringList lines{output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)};
    bool sawHeader{false};

    for (const QString &line : lines) {
        const QString clean{QString{line}.remove(ansi).trimmed()};

        if (clean.isEmpty()) {
            continue;
        }

        if (!sawHeader) {
            sawHeader = clean.startsWith(QLatin1String("NAME"));
            continue;
        }

        const QStringList columns{clean.split(whitespace, Qt::SkipEmptyParts)};

        if (!columns.isEmpty()) {
            names.append(columns.first());
        }
    }

    return names;
}

QStringList KapsuleDetector::emptyListFallback() const
{
    // With no containers yet, offer the configured default
    // container, which entering creates (as Konsole does).
    return {defaultContainerName()};
}

QString KapsuleDetector::defaultContainerName()
{
    // The kapsule daemon resolves "default_container" from INI files,
    // most specific first; its compiled-in default is "kapsule" and
    // the packaged configuration sets "container".
    const QString configHome{QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)};
    const QStringList candidates{
        configHome + QStringLiteral("/kapsule/kapsule.conf"),
        configHome + QStringLiteral("/kapsule.conf"),
        QStringLiteral("/etc/kapsule/kapsule.conf"),
        QStringLiteral("/usr/lib/kapsule/kapsule.conf"),
    };

    for (const QString &path : candidates) {
        if (!QFileInfo::exists(path)) {
            continue;
        }

        const QSettings settings{path, QSettings::IniFormat};
        const QString name{settings.value(QStringLiteral("kapsule/default_container")).toString().trimmed()};

        if (!name.isEmpty()) {
            return name;
        }
    }

    return QStringLiteral("kapsule");
}
}
