# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: GPL-2.0-or-later

# The root helper of the activation for all users. KAuth starts it through the
# system bus once polkit has authorized the action, so it needs the D-Bus policy
# and service files and the polkit actions installed on the host: a
# distribution package, not a Flatpak, which cannot install any of them.

add_executable(fontmatrix_helper fontmatrixhelper.cpp)
target_link_libraries(fontmatrix_helper KF6::AuthCore Qt6::Core)
set_source_files_properties(fontmatrixhelper.cpp PROPERTIES COMPILE_DEFINITIONS "QT_NO_CAST_FROM_ASCII")

install(TARGETS fontmatrix_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(fontmatrix_helper org.fontmatrix root)
kauth_install_actions(org.fontmatrix org.fontmatrix.actions)
