// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ToolboxDetector.h"

#include <KLocalizedString>
#include <QRegularExpression>

namespace Kwi
{
ToolboxDetector::ToolboxDetector(QObject *parent)
    : ShellContainerDetector{parent}
{
}

QString ToolboxDetector::typeId() const
{
    return QStringLiteral("toolbox");
}

QString ToolboxDetector::displayName() const
{
    return i18nc("@title:menu container type", "Toolbox");
}

QString ToolboxDetector::iconName() const
{
    return QStringLiteral("utilities-terminal");
}

QStringList ToolboxDetector::entryCommand(const QString &containerName) const
{
    return {QStringLiteral("toolbox"), QStringLiteral("enter"), containerName};
}

QStringList ToolboxDetector::listCommand() const
{
    return {QStringLiteral("toolbox"), QStringLiteral("list"), QStringLiteral("--containers")};
}

QStringList ToolboxDetector::parseNames(const QString &output) const
{
    // Adapted from Konsole's ToolboxListParser (GPL-2.0-or-later):
    // "toolbox list --containers" prints a header row starting with
    // "CONTAINER ID" (or "ID"), then one row per container with the
    // name in the second whitespace-separated column.
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

        if (!sawHeader) {
            if (lower.startsWith(QLatin1String("container id")) || lower.startsWith(QLatin1String("id"))) {
                sawHeader = true;
            }

            continue;
        }

        const QStringList columns{trimmed.split(whitespace, Qt::SkipEmptyParts)};

        if (columns.size() >= 2) {
            names.append(columns[1]);
        }
    }

    return names;
}
}
