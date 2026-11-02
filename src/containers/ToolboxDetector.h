// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "ShellContainerDetector.h"

namespace Kwi
{
class ToolboxDetector : public ShellContainerDetector
{
    Q_OBJECT

public:
    explicit ToolboxDetector(QObject *parent = nullptr);

    [[nodiscard]] QString typeId() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString iconName() const override;
    [[nodiscard]] QStringList entryCommand(const QString &containerName) const override;

protected:
    [[nodiscard]] QStringList listCommand() const override;
    [[nodiscard]] QStringList parseNames(const QString &output) const override;
};
}
