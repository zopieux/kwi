// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// The distrobox and toolbox tables are adapted from Konsole's
// ContainerDetectorParsingTest (GPL-2.0-or-later).

#include "containers/ContainerInfo.h"
#include "containers/DistroboxDetector.h"
#include "containers/KapsuleDetector.h"
#include "containers/ToolboxDetector.h"
#include "settings/LaunchSettings.h"

#include <QTest>

namespace
{
// The parsers are protected: they are exercised through the tools'
// output in production. A subclass re-exports one for the tables.
class ParsingToolboxDetector final : public Kwi::ToolboxDetector
{
public:
    using Kwi::ToolboxDetector::parseNames;
};

class ParsingDistroboxDetector final : public Kwi::DistroboxDetector
{
public:
    using Kwi::DistroboxDetector::parseNames;
};

class ParsingKapsuleDetector final : public Kwi::KapsuleDetector
{
public:
    using Kwi::KapsuleDetector::parseNames;
};
}

namespace Kwi
{
class ContainerParsingTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parseDistrobox_data();
    void parseDistrobox();
    void parseToolbox_data();
    void parseToolbox();
    void parseKapsule_data();
    void parseKapsule();
    void containerIds_data();
    void containerIds();
    void presetContainerValidation();
};

void ContainerParsingTest::parseDistrobox_data()
{
    QTest::addColumn<QString>("output");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("pipe-table-valid") << QStringLiteral(
        "ID           | NAME                 | STATUS             | IMAGE\n"
        "f5678ef7cf1d | Android-Development  | Up 11 minutes      | quay.io/toolbx-images/debian-toolbox:latest\n"
        "997f5777978a | pmos                 | Up 11 minutes      | docker.io/library/debian:latest\n")
                                      << QStringList({QStringLiteral("Android-Development"), QStringLiteral("pmos")});

    QTest::newRow("whitespace-table-valid") << QStringLiteral(
        "ID NAME STATUS IMAGE\n"
        "abc123 fedora-41 Up registry.fedoraproject.org/fedora-toolbox:41\n")
                                            << QStringList({QStringLiteral("fedora-41")});

    QTest::newRow("error-text") << QStringLiteral("Error: no containers found for current runtime\n") << QStringList{};

    QTest::newRow("missing-header") << QStringLiteral("f5678ef7cf1d | Android-Development | Up 11 minutes | quay.io/toolbx-images/debian-toolbox:latest\n")
                                    << QStringList{};

    QTest::newRow("header-only") << QStringLiteral("ID           | NAME                 | STATUS             | IMAGE\n") << QStringList{};

    QTest::newRow("empty") << QString() << QStringList{};

    QTest::newRow("whitespace-only") << QStringLiteral("   \n\t  \n") << QStringList{};
}

void ContainerParsingTest::parseDistrobox()
{
    QFETCH(QString, output);
    QFETCH(QStringList, expected);
    const ParsingDistroboxDetector detector;
    QCOMPARE(detector.parseNames(output), expected);
}

void ContainerParsingTest::parseToolbox_data()
{
    QTest::addColumn<QString>("output");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("toolbox-valid") << QStringLiteral(
        "CONTAINER ID  CONTAINER NAME  CREATED       STATUS   IMAGE NAME\n"
        "ecd075209720  codex           17 hours ago  running  registry.fedoraproject.org/fedora-toolbox:latest\n"
        "24451bedace8  tuxedo-noble    8 minutes ago running  docker.io/library/ubuntu:24.04\n")
                                   << QStringList({QStringLiteral("codex"), QStringLiteral("tuxedo-noble")});

    QTest::newRow("short-header-valid") << QStringLiteral(
        "ID NAME STATUS IMAGE\n"
        "abcd1234 toolbox-fedora running registry.fedoraproject.org/fedora-toolbox:latest\n")
                                        << QStringList({QStringLiteral("toolbox-fedora")});

    QTest::newRow("error-text") << QStringLiteral("Error: toolbox command failed\n") << QStringList{};

    QTest::newRow("missing-header") << QStringLiteral("ecd075209720 codex 17-hours running registry.fedoraproject.org/fedora-toolbox:latest\n")
                                    << QStringList{};

    QTest::newRow("header-only") << QStringLiteral("CONTAINER ID  CONTAINER NAME  CREATED       STATUS   IMAGE NAME\n") << QStringList{};

    QTest::newRow("explicit-empty-string") << QStringLiteral("") << QStringList{};

    QTest::newRow("whitespace-only") << QStringLiteral(" \n\t\n ") << QStringList{};
}

void ContainerParsingTest::parseToolbox()
{
    QFETCH(QString, output);
    QFETCH(QStringList, expected);
    const ParsingToolboxDetector detector;
    QCOMPARE(detector.parseNames(output), expected);
}

void ContainerParsingTest::parseKapsule_data()
{
    QTest::addColumn<QString>("output");
    QTest::addColumn<QStringList>("expected");

    QTest::newRow("styled-table-valid") << QStringLiteral(
        "\x1B[1mNAME                STATUS      IMAGE                    MODE        CREATED\x1B[0m\n"
        "\x1B[32mdevbox              Running\x1B[0m     images:archlinux         session     2026-10-30\n"
        "\x1B[31moldbox              Stopped\x1B[0m     images:ubuntu/24.04      session     2026-09-14\n")
                                        << QStringList({QStringLiteral("devbox"), QStringLiteral("oldbox")});

    QTest::newRow("plain-table-valid") << QStringLiteral(
        "NAME                STATUS      IMAGE                    MODE        CREATED\n"
        "devbox              Running     images:archlinux         session     2026-10-30\n")
                                       << QStringList({QStringLiteral("devbox")});

    QTest::newRow("no-containers") << QStringLiteral("No containers found.\n") << QStringList{};

    QTest::newRow("header-only") << QStringLiteral("NAME                STATUS      IMAGE                    MODE        CREATED\n") << QStringList{};

    QTest::newRow("error-text") << QStringLiteral("Failed to connect to kapsule daemon\n") << QStringList{};

    QTest::newRow("empty") << QString() << QStringList{};
}

void ContainerParsingTest::parseKapsule()
{
    QFETCH(QString, output);
    QFETCH(QStringList, expected);
    const ParsingKapsuleDetector detector;
    QCOMPARE(detector.parseNames(output), expected);
}

void ContainerParsingTest::containerIds_data()
{
    QTest::addColumn<QString>("id");
    QTest::addColumn<bool>("valid");

    QTest::newRow("toolbox") << QStringLiteral("toolbox:fedora-41") << true;
    QTest::newRow("distrobox") << QStringLiteral("distrobox:debian") << true;
    QTest::newRow("kapsule") << QStringLiteral("kapsule:devbox") << true;
    QTest::newRow("colon-in-name") << QStringLiteral("distrobox:ubuntu:24.04") << true;
    QTest::newRow("unknown-type") << QStringLiteral("podman:x") << false;
    QTest::newRow("empty-name") << QStringLiteral("toolbox:") << false;
    QTest::newRow("empty-type") << QStringLiteral(":x") << false;
    QTest::newRow("space-in-name") << QStringLiteral("toolbox:my box") << false;
    QTest::newRow("no-separator") << QStringLiteral("toolbox") << false;
    QTest::newRow("empty") << QString() << false;
}

void ContainerParsingTest::containerIds()
{
    QFETCH(QString, id);
    QFETCH(bool, valid);
    QCOMPARE(isValidContainerId(id), valid);

    if (valid) {
        const auto split{splitContainerId(id)};
        QVERIFY(split.has_value());
        QCOMPARE(containerId(split->type, split->name), id);
    }
}

void ContainerParsingTest::presetContainerValidation()
{
    LaunchSettings settings;
    settings.container = QStringLiteral("kapsule:devbox");
    QVERIFY(validateLaunchSettings(settings));

    settings.container = QStringLiteral("podman:x");
    QVERIFY(!validateLaunchSettings(settings));

    settings.container = QStringLiteral("not-an-id");
    QVERIFY(!validateLaunchSettings(settings));

    settings.container = QString{};
    QVERIFY(validateLaunchSettings(settings));
}

} // namespace Kwi

QTEST_GUILESS_MAIN(Kwi::ContainerParsingTest)

#include "ContainerParsingTest.moc"
