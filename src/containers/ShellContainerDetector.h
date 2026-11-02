// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ContainerDetector.h"

#include <QHash>
#include <QList>
#include <memory>

class QProcess;

namespace Kwi
{
/**
 * Base class for detectors whose tool is a command-line program with a
 * list subcommand. Subclasses provide the list command and a parser
 * for its output; this class runs the command, on the host when Kwi
 * itself runs inside a Flatpak, and reports the parsed names.
 */
class ShellContainerDetector : public ContainerDetector
{
    Q_OBJECT

public:
    explicit ShellContainerDetector(QObject *parent = nullptr);
    ~ShellContainerDetector() override;

    void startListContainers() override;

protected:
    /** Program and arguments that print this tool's container list. */
    [[nodiscard]] virtual QStringList listCommand() const = 0;

    /** Extract container names from the list command's output. */
    [[nodiscard]] virtual QStringList parseNames(const QString &output) const = 0;

    [[nodiscard]] virtual QStringList emptyListFallback() const;

private:
    void finishList(const QString &output);

    std::unique_ptr<QProcess> m_process{};
    bool m_listingRan{false};
};
}
