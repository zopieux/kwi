// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "ShellContainerDetector.h"

#include <KSandbox>
#include <QProcess>
#include <QStandardPaths>

namespace Kwi
{
ShellContainerDetector::ShellContainerDetector(QObject *parent)
    : ContainerDetector{parent}
{
}

ShellContainerDetector::~ShellContainerDetector() = default;

void ShellContainerDetector::startListContainers()
{
    if (m_process) {
        // A listing is already in flight; its result serves both calls.
        return;
    }

    m_listingRan = false;

    QStringList command{listCommand()};

    if (command.isEmpty()) {
        Q_EMIT listContainersFinished({});
        return;
    }

    const QString program{command.takeFirst()};

    if (KSandbox::isFlatpak()) {
        // The tool lives on the host, not in the sandbox. flatpak-spawn
        // is part of Flatpak itself, so it is always present here; the
        // manifest grants the org.freedesktop.Flatpak talk permission
        // it needs.
        if (QStandardPaths::findExecutable(QStringLiteral("flatpak-spawn")).isEmpty()) {
            Q_EMIT listContainersFinished({});
            return;
        }

        QStringList arguments{QStringLiteral("--host"), program};
        arguments += command;

        m_process = std::make_unique<QProcess>(this);
        m_process->setProgram(QStringLiteral("flatpak-spawn"));
        m_process->setArguments(arguments);
    } else {
        if (QStandardPaths::findExecutable(program).isEmpty()) {
            Q_EMIT listContainersFinished({});
            return;
        }

        m_process = std::make_unique<QProcess>(this);
        m_process->setProgram(program);
        m_process->setArguments(command);
    }

    connect(m_process.get(), &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        QString output;

        // Only a list command that actually ran to success counts as
        // an empty listing. Through flatpak-spawn a tool missing on
        // the host is indistinguishable from one that ran, except by
        // its exit code; treating its failure as "no containers"
        // would offer fallback entries for tools that do not exist.
        m_listingRan = status == QProcess::NormalExit && exitCode == 0;

        if (m_listingRan) {
            output = QString::fromUtf8(m_process->readAllStandardOutput());
        }

        m_process.reset();
        finishList(output);
    });
    connect(m_process.get(), &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            m_listingRan = false;
            m_process.reset();
            finishList({});
        }
    });

    m_process->start();
}

QStringList ShellContainerDetector::emptyListFallback() const
{
    return {};
}

void ShellContainerDetector::finishList(const QString &output)
{
    QStringList names{output.isEmpty() ? QStringList{} : parseNames(output)};

    if (names.isEmpty() && m_listingRan) {
        names = emptyListFallback();
    }

    Q_EMIT listContainersFinished(names);
}

}
