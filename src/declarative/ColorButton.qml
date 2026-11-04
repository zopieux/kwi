// SPDX-FileCopyrightText: 2026 Eike Hein <hein@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs as Dialogs
import org.kde.ki18n
import org.kde.kirigami as Kirigami

Controls.Button {
    id: button

    required property color color
    property string dialogTitle

    signal accepted(color color)

    Accessible.description: KI18n.i18nc("@info:accessibility", "Current color: %1. Opens a color chooser.", color)

    implicitWidth: Kirigami.Units.gridUnit * 3

    Rectangle {
        anchors.centerIn: parent
        width: Math.max(0, button.width - Kirigami.Units.smallSpacing * 2)
        height: Math.max(0, button.height - Kirigami.Units.smallSpacing * 2)
        color: button.color
        border.color: Kirigami.Theme.disabledTextColor
    }

    onClicked: {
        picker.selectedColor = color;
        picker.open();
    }

    Dialogs.ColorDialog {
        id: picker

        title: button.dialogTitle

        onAccepted: button.accepted(selectedColor)
    }
}
