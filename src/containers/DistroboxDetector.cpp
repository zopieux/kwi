// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DistroboxDetector.h"

#include <KLocalizedString>
#include <QRegularExpression>

namespace Kwi
{
DistroboxDetector::DistroboxDetector(QObject *parent)
    : ShellContainerDetector{parent}
{
}

QString DistroboxDetector::typeId() const
{
    return QStringLiteral("distrobox");
}

QString DistroboxDetector::displayName() const
{
    return i18nc("@title:menu container type", "Distrobox");
}

QString DistroboxDetector::iconName() const
{
    return QStringLiteral("utilities-terminal");
}

QStringList DistroboxDetector::entryCommand(const QString &containerName) const
{
    return {QStringLiteral("distrobox"), QStringLiteral("enter"), containerName};
}

QStringList DistroboxDetector::listCommand() const
{
    return {QStringLiteral("distrobox"), QStringLiteral("list")};
}

QStringList DistroboxDetector::parseNames(const QString &output) const
{
    // Adapted from Konsole's DistroboxListParser (GPL-2.0-or-later):
    // "distrobox list" prints a header row starting with "ID", then
    // one row per container with pipe-separated columns; the name is
    // the second column. Some versions fall back to whitespace
    // columns, handled the same way as the Toolbox output.
    QStringList names;
    const QStringList lines{output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)};
    static const QRegularExpression whitespace{QStringLiteral("\\s+")};

    bool sawHeader{false};

    for (const QString &line : lines) {
        const QString trimmed{line.trimmed()};

        if (trimmed.isEmpty()) {
            continue;
        }

        const QString lower{trimmed.toLower()};

        if (lower.startsWith(QLatin1String("id")) || lower.startsWith(QLatin1String("container"))) {
            sawHeader = true;
            continue;
        }

        if (!sawHeader) {
            continue;
        }

        QString containerName;
        const QStringList pipeColumns{trimmed.split(QLatin1Char('|'))};

        if (pipeColumns.size() >= 2) {
            containerName = pipeColumns[1].trimmed();
        } else {
            const QStringList columns{trimmed.split(whitespace, Qt::SkipEmptyParts)};

            if (columns.size() >= 2) {
                containerName = columns[1].trimmed();
            }
        }

        if (!containerName.isEmpty()) {
            names.append(containerName);
        }
    }

    return names;
}
}
