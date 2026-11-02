// SPDX-FileCopyrightText: 2026 Christian McHugh
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "containers/ContainerRegistry.h"
#include "containers/ContainerInfo.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace
{
QString writeScript(const QDir &dir, const QString &name, const QByteArray &content)
{
    const QString path{dir.filePath(name)};
    QFile script{path};
    script.open(QIODevice::WriteOnly);
    script.write(content);
    script.close();
    QFile::setPermissions(path, QFileDevice::ExeUser | QFileDevice::ReadUser | QFileDevice::WriteUser);
    return path;
}

bool waitForRefresh(Kwi::ContainerRegistry &registry)
{
    QSignalSpy spy{&registry, &Kwi::ContainerRegistry::containersChanged};
    return spy.wait(10000);
}

QStringList menuLabels(const Kwi::ContainerRegistry &registry)
{
    QStringList labels;

    for (const QVariant &entry : registry.menuEntries()) {
        labels += entry.toMap().value(QStringLiteral("label")).toString();
    }

    return labels;
}

QStringList menuContainerNames(const Kwi::ContainerRegistry &registry, const QString &type)
{
    QStringList names;

    for (const QVariant &entry : registry.menuEntries()) {
        const QVariantMap map{entry.toMap()};

        if (map.value(QStringLiteral("kind")).toString() == QStringLiteral("container") && map.value(QStringLiteral("type")).toString() == type) {
            names += map.value(QStringLiteral("name")).toString();
        }
    }

    return names;
}
}

namespace Kwi
{
class ContainerRegistryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void menuAndEntryCommands();
    void kapsuleDefaultOfferedWhenListEmpty();
    void kapsuleDefaultNotOfferedWhenListFails();

private:
    QTemporaryDir m_fakeBin;
    QByteArray m_originalPath;
};

void ContainerRegistryTest::initTestCase()
{
    const QDir bin{m_fakeBin.path()};
    writeScript(bin,
                QStringLiteral("toolbox"),
                "#!/bin/sh\n"
                "if [ \"$1\" = \"list\" ]; then\n"
                "cat <<'OUT'\n"
                "CONTAINER ID  CONTAINER NAME  CREATED       STATUS      IMAGE NAME\n"
                "abc123        fedora-41       2026-01-01    running     registry.fedoraproject.org/fedora-toolbox:41\n"
                "def456        krunner-test    2026-01-02    exited      localhost/krunner-test:latest\n"
                "OUT\n"
                "fi\n");
    writeScript(bin,
                QStringLiteral("distrobox"),
                "#!/bin/sh\n"
                "if [ \"$1\" = \"list\" ]; then\n"
                "cat <<'OUT'\n"
                "ID           | NAME                 | STATUS             | IMAGE\n"
                "aaa111       | debian               | running            | docker.io/library/debian:12\n"
                "bbb222       | okular-dev           | exited             | localhost/okular-dev:latest\n"
                "OUT\n"
                "fi\n");
    writeScript(bin,
                QStringLiteral("kapsule"),
                "#!/bin/sh\n"
                "if [ \"$1\" = \"list\" ]; then\n"
                "printf '\\033[1mNAME                STATUS      IMAGE                    MODE        CREATED\\033[0m\\n'\n"
                "printf '\\033[32mdevbox              Running\\033[0m     images:archlinux         session     2026-10-30\\n'\n"
                "fi\n");

    m_originalPath = qgetenv("PATH");
    qputenv("PATH", (m_fakeBin.path() + QLatin1Char(':') + QString::fromLocal8Bit(m_originalPath)).toUtf8());
}

void ContainerRegistryTest::cleanupTestCase()
{
    qputenv("PATH", m_originalPath);
}

void ContainerRegistryTest::menuAndEntryCommands()
{
    ContainerRegistry registry;
    QVERIFY(waitForRefresh(registry));

    // Copy-initialization on purpose: QVariantList{list} would build a
    // one-element list holding the whole list as a single QVariant.
    const QVariantList entries = registry.menuEntries();
    QCOMPARE(entries.size(), 10);
    QCOMPARE(entries.at(0).toMap().value(QStringLiteral("kind")).toString(), QStringLiteral("header"));
    QCOMPARE(entries.at(1).toMap().value(QStringLiteral("kind")).toString(), QStringLiteral("host"));

    const QStringList labels{menuLabels(registry)};
    QVERIFY(labels.contains(QStringLiteral("Toolbox")));
    QVERIFY(labels.contains(QStringLiteral("Distrobox")));
    QVERIFY(labels.contains(QStringLiteral("Kapsule")));
    QCOMPARE(menuContainerNames(registry, QStringLiteral("toolbox")), QStringList({QStringLiteral("fedora-41"), QStringLiteral("krunner-test")}));
    QCOMPARE(menuContainerNames(registry, QStringLiteral("distrobox")), QStringList({QStringLiteral("debian"), QStringLiteral("okular-dev")}));
    QCOMPARE(menuContainerNames(registry, QStringLiteral("kapsule")), QStringList({QStringLiteral("devbox")}));

    QCOMPARE(registry.entryCommandForId(QStringLiteral("toolbox:krunner-test")),
             QStringList({QStringLiteral("toolbox"), QStringLiteral("enter"), QStringLiteral("krunner-test")}));
    QCOMPARE(registry.entryCommandForId(QStringLiteral("distrobox:ubuntu:24.04")),
             QStringList({QStringLiteral("distrobox"), QStringLiteral("enter"), QStringLiteral("ubuntu:24.04")}));
    QCOMPARE(registry.entryCommandForId(QStringLiteral("kapsule:devbox")),
             QStringList({QStringLiteral("kapsule"), QStringLiteral("enter"), QStringLiteral("devbox")}));
    QVERIFY(registry.entryCommandForId(QStringLiteral("podman:x")).isEmpty());
    QVERIFY(registry.entryCommandForId(QStringLiteral("not-an-id")).isEmpty());
    QVERIFY(registry.entryCommandForId(QString{}).isEmpty());
}

void ContainerRegistryTest::kapsuleDefaultOfferedWhenListEmpty()
{
    QTemporaryDir shadowBin;
    writeScript(QDir{shadowBin.path()}, QStringLiteral("kapsule"), "#!/bin/sh\necho 'No containers found.'\n");
    const QByteArray pathBefore{qgetenv("PATH")};
    qputenv("PATH", (shadowBin.path() + QLatin1Char(':') + QString::fromLocal8Bit(pathBefore)).toUtf8());

    ContainerRegistry registry;
    const bool refreshed{waitForRefresh(registry)};
    const QStringList names{refreshed ? menuContainerNames(registry, QStringLiteral("kapsule")) : QStringList{}};
    qputenv("PATH", pathBefore);

    QVERIFY(refreshed);
    QCOMPARE(names.size(), 1);
    // The resolved name comes from the kapsule configuration on the
    // machine running the test: the packaged default or the
    // compiled-in one.
    QVERIFY(names.constFirst() == QStringLiteral("kapsule") || names.constFirst() == QStringLiteral("container"));
}

void ContainerRegistryTest::kapsuleDefaultNotOfferedWhenListFails()
{
    // A list command that fails (as flatpak-spawn reports a tool
    // missing on the host) is not an empty listing: no default
    // container may be offered for a tool that is not there.
    QTemporaryDir shadowBin;
    writeScript(QDir{shadowBin.path()}, QStringLiteral("kapsule"), "#!/bin/sh\nexit 1\n");
    const QByteArray pathBefore{qgetenv("PATH")};
    qputenv("PATH", (shadowBin.path() + QLatin1Char(':') + QString::fromLocal8Bit(pathBefore)).toUtf8());

    ContainerRegistry registry;
    const bool refreshed{waitForRefresh(registry)};
    const QStringList names{refreshed ? menuContainerNames(registry, QStringLiteral("kapsule")) : QStringList{QStringLiteral("unrefreshed")}};
    qputenv("PATH", pathBefore);

    QVERIFY(refreshed);
    QVERIFY(names.isEmpty());
}

} // namespace Kwi

QTEST_GUILESS_MAIN(Kwi::ContainerRegistryTest)

#include "ContainerRegistryTest.moc"
