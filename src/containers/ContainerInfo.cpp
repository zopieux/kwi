// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ContainerInfo.h"

#include <QChar>
#include <algorithm>

namespace Kwi
{
bool ContainerInfo::isValid() const
{
    return isValidContainerId(id());
}

QString ContainerInfo::id() const
{
    return containerId(type, name);
}

QStringList containerTypes()
{
    // Keep in sync with the detectors registered by ContainerRegistry.
    return {QStringLiteral("toolbox"), QStringLiteral("distrobox"), QStringLiteral("kapsule")};
}

QString containerId(const QString &type, const QString &name)
{
    if (type.isEmpty() || name.isEmpty()) {
        return {};
    }

    return type + QLatin1Char(':') + name;
}

std::optional<ContainerInfo> splitContainerId(const QString &id)
{
    const qsizetype separator{id.indexOf(QLatin1Char(':'))};

    if (separator <= 0 || separator == id.size() - 1) {
        return std::nullopt;
    }

    return ContainerInfo{.type = id.left(separator), .name = id.mid(separator + 1)};
}

bool isValidContainerId(const QString &id)
{
    const auto info{splitContainerId(id)};

    if (!info || !containerTypes().contains(info->type)) {
        return false;
    }

    // The name may itself contain ':' (a distrobox named after its
    // image, e.g. "ubuntu:24.04"); the identifier splits on the first
    // ':' only, like Konsole's key handling. Whitespace cannot appear
    // in a listed name, so it marks a typed identifier as malformed.
    return std::ranges::none_of(info->name, [](QChar c) {
        return c.isSpace();
    });
}
}
