// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QString>
#include <QStringList>
#include <optional>

namespace Kwi
{
/**
 * A container a session can be started in, identified by the type of
 * container tool managing it and the container's name, e.g. the
 * identifier string "distrobox:okular-dev".
 */
struct ContainerInfo {
    QString type{};
    QString name{};

    [[nodiscard]] bool isValid() const;
    [[nodiscard]] QString id() const;
};

/** The container tool types this build knows how to enter. */
[[nodiscard]] QStringList containerTypes();

/** Compose the identifier string for a container ("<type>:<name>"). */
[[nodiscard]] QString containerId(const QString &type, const QString &name);

/** Split an identifier string into its type and name, if well-formed. */
[[nodiscard]] std::optional<ContainerInfo> splitContainerId(const QString &id);

/**
 * Whether an identifier string names a container this build can enter:
 * a known type, and a non-empty name without whitespace. A name may
 * itself contain ':'; the split is on the first ':' only.
 * The empty string is not an identifier; it means "on the host".
 */
[[nodiscard]] bool isValidContainerId(const QString &id);
}
