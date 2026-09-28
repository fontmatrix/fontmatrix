kdoctools_create_handbook(
    index.docbook
    INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/en
    SUBDIR fontmatrix
)

# Dev-tree mirror: copy the handbook artifacts into ${CMAKE_BINARY_DIR}/data/
# under the layout khelpcenter expects (doc/HTML/<lang>/<app>/). Combined with
# the launcher in src/CMakeLists.txt this lets F1 find the handbook when the
# binary is run from the build tree, without `make install`.
set(_handbook_dev_dir "${CMAKE_BINARY_DIR}/data/doc/HTML/en/fontmatrix")
add_custom_command(
    OUTPUT  "${_handbook_dev_dir}/index.cache.bz2"
            "${_handbook_dev_dir}/index.docbook"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_handbook_dev_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2"
            "${_handbook_dev_dir}/index.cache.bz2"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/index.docbook"
            "${_handbook_dev_dir}/index.docbook"
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2"
            "${CMAKE_CURRENT_SOURCE_DIR}/index.docbook"
    COMMENT "Mirroring handbook into ${_handbook_dev_dir}"
    VERBATIM
)
add_custom_target(fontmatrix-handbook-devtree ALL
    DEPENDS "${_handbook_dev_dir}/index.cache.bz2"
            "${_handbook_dev_dir}/index.docbook"
)

# The same handbook as HTML, for the desktops that have no KHelpCenter: a
# Flatpak, where nothing of the application is visible to the host, Windows,
# macOS, and a Linux where it is simply not installed. meinproc writes one page
# per chapter into ${_handbook_html_dir}; handbook-html.cmake then cuts the
# links to KHelpCenter's own stylesheets, which no browser can follow.
set(_handbook_html_dir "${CMAKE_CURRENT_BINARY_DIR}/html")
add_custom_command(
    OUTPUT  "${_handbook_html_dir}/index.html"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_handbook_html_dir}"
    COMMAND ${CMAKE_COMMAND} -E chdir "${_handbook_html_dir}"
            "$<TARGET_FILE:${KDOCTOOLS_MEINPROC_EXECUTABLE}>"
            "${CMAKE_CURRENT_SOURCE_DIR}/index.docbook"
    COMMAND ${CMAKE_COMMAND} "-DHANDBOOK_DIR=${_handbook_html_dir}"
            -P "${CMAKE_SOURCE_DIR}/cmake/handbook-html.cmake"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/index.docbook"
    COMMENT "Generating the browsable handbook"
    VERBATIM
)

# and the same mirror, so that F1 works from the build tree
set(_handbook_html_dev_dir "${CMAKE_BINARY_DIR}/share/fontmatrix/handbook/en")
add_custom_command(
    OUTPUT  "${_handbook_html_dev_dir}/index.html"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_handbook_html_dev_dir}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${_handbook_html_dir}" "${_handbook_html_dev_dir}"
    DEPENDS "${_handbook_html_dir}/index.html"
    COMMENT "Mirroring the browsable handbook into ${_handbook_html_dev_dir}"
    VERBATIM
)
add_custom_target(fontmatrix-handbook-html ALL
    DEPENDS "${_handbook_html_dev_dir}/index.html"
)

install(DIRECTORY "${_handbook_html_dir}/"
        DESTINATION "${KDE_INSTALL_DATADIR}/fontmatrix/handbook/en"
        FILES_MATCHING PATTERN "*.html")
