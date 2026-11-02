// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ContainerInfo.h"

#include <QObject>
#include <QString>
#include <QStringList>

namespace Kwi
{
/**
 * Interface for one kind of container tool (Toolbox, Distrobox, ...).
 *
 * A detector knows how to list the containers its tool manages on this
 * machine and how to build the command that starts a shell inside one
 * of them. Listing is asynchronous: call startListContainers() and
 * wait for listContainersFinished(). A tool that is not installed, or
 * whose listing fails, reports an empty list.
 */
class ContainerDetector : public QObject
{
    Q_OBJECT

public:
    explicit ContainerDetector(QObject *parent = nullptr)
        : QObject{parent}
    {
    }

    ~ContainerDetector() override = default;

    /** Unique identifier of the container type, e.g. "toolbox". */
    [[nodiscard]] virtual QString typeId() const = 0;

    /** Human-readable name for display in the UI, e.g. "Toolbox". */
    [[nodiscard]] virtual QString displayName() const = 0;

    /** Icon name for UI representation. */
    [[nodiscard]] virtual QString iconName() const = 0;

    /**
     * The program and arguments that enter the named container,
     * e.g. {"toolbox", "enter", "fedora-41"}. The command starts the
     * container's shell, replacing the session's own command.
     */
    [[nodiscard]] virtual QStringList entryCommand(const QString &containerName) const = 0;

    /** Start an asynchronous listing of this tool's containers. */
    virtual void startListContainers() = 0;

Q_SIGNALS:
    /** The names of the containers found, possibly empty. */
    void listContainersFinished(const QStringList &containerNames);
};
}
