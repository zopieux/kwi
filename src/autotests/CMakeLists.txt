# SPDX-FileCopyrightText: 2026 Christian McHugh
# SPDX-License-Identifier: CC0-1.0

include(ECMAddTests)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)

ecm_add_test(
    ContainerParsingTest.cpp
    LINK_LIBRARIES kwi-core Qt::Test
)

ecm_add_test(
    ContainerRegistryTest.cpp
    LINK_LIBRARIES kwi-core Qt::Test
)
