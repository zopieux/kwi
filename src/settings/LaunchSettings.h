// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ProcessLaunch.h"
#include <QString>
#include <QStringList>
#include <optional>

namespace Kwi
{
struct LaunchSettings {
    enum class DirectoryMode {
        Current,
        Home,
        Custom
    };

    QStringList command{};
    DirectoryMode directoryMode{DirectoryMode::Current};
    QString directory{};
    QStringList environment{};
    /** Container identifier ("<type>:<name>") to start in; empty runs on the host. */
    QString container{};
    /** Start in the container of the currently active session, if it is in one. */
    bool inheritContainerContext{false};

    bool operator==(const LaunchSettings &) const = default;
};

struct LaunchRequest {
    std::optional<QStringList> command{};
    QString directory{};
    QString contextDirectory{};
    /** Disengaged uses the preset's container; engaged (even empty) overrides it. */
    std::optional<QString> container{};
};

[[nodiscard]] bool validateLaunchSettings(const LaunchSettings &settings, QString *error = nullptr);
bool validateEnvironment(const QStringList &entries, QString *error = nullptr);
[[nodiscard]] TerminalLaunch resolveLaunch(const LaunchSettings &settings, const LaunchRequest &request = {});
}
