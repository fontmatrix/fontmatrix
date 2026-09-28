# SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
# SPDX-License-Identifier: GPL-2.0-or-later

# Makes the HTML meinproc writes stand on its own. Its pages ask KHelpCenter for
# the stylesheets and the header image (help:/kdoctools6-common/…), a scheme no
# browser knows; the links go, and a stylesheet of our own takes their place, so
# that the handbook opens from a file manager, a browser or a sandbox with
# nothing else installed.
#
# Called from doc/fontmatrix/CMakeLists.txt with -DHANDBOOK_DIR=<directory>.

if(NOT HANDBOOK_DIR)
    message(FATAL_ERROR "handbook-html.cmake needs -DHANDBOOK_DIR=<directory>")
endif()

set(_style [[<style type="text/css">
body { background: #fff; color: #232629; font-family: sans-serif; line-height: 1.55;
       margin: 0 auto; max-width: 54em; padding: 1.5em 2em 5em; }
h1, h2, h3, h4 { color: #1f2a33; line-height: 1.25; margin-top: 1.6em; }
h1 { font-size: 1.8em; } h2 { font-size: 1.4em; } h3 { font-size: 1.15em; }
a { color: #2980b9; } a:visited { color: #6d7f8b; }
code, pre, tt, .command, .filename, .userinput { font-family: monospace; font-style: normal; }
pre { background: #f6f7f8; border: 1px solid #dde1e4; border-radius: 3px;
      overflow-x: auto; padding: .6em .9em; }
.guimenu, .guimenuitem, .guisubmenu, .guibutton, .guilabel, .interface { font-weight: 600; }
.keycap { border: 1px solid #c8ced3; border-radius: 3px; padding: 0 .3em; }
dt { font-weight: 600; margin-top: .7em; }
table { border-collapse: collapse; }
div.navheader table, div.navfooter table { font-size: .9em; width: 100%; }
div.navfooter { border-top: 1px solid #dde1e4; margin-top: 3em; }
div.navheader { border-bottom: 1px solid #dde1e4; margin-bottom: 2em; }
div.note, div.tip, div.warning { background: #f6f7f8; border-left: 4px solid #2980b9;
                                 margin: 1.2em 0; padding: .6em 1em; }
div.warning { border-left-color: #c0392b; }
@media (prefers-color-scheme: dark) {
  body { background: #1b1e20; color: #d7dade; }
  h1, h2, h3, h4 { color: #eff0f1; }
  a { color: #5ea6dd; } a:visited { color: #9aa7b0; }
  pre, div.note, div.tip, div.warning { background: #24282b; border-color: #3b4045; }
  div.navheader, div.navfooter { border-color: #3b4045; }
}
</style>]])

file(GLOB _pages "${HANDBOOK_DIR}/*.html")
foreach(_page IN LISTS _pages)
    file(READ "${_page}" _html)
    string(REGEX REPLACE "<link[^>]*help:/kdoctools6-common/[^>]*>" "" _html "${_html}")
    string(REGEX REPLACE "<img[^>]*help:/kdoctools6-common/[^>]*>" "" _html "${_html}")
    # the licences the credits chapter links to are KHelpCenter's copies; the
    # texts themselves are where they have always been
    string(REPLACE "help:/kdoctools6-common/gpl-license.html"
                   "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html" _html "${_html}")
    string(REPLACE "help:/kdoctools6-common/fdl-license.html"
                   "https://www.gnu.org/licenses/fdl-1.2.html" _html "${_html}")
    string(REPLACE "help:/kdoctools6-common/fdl-notice.html"
                   "https://www.gnu.org/licenses/fdl-1.2.html" _html "${_html}")
    string(REPLACE "</head>" "${_style}</head>" _html "${_html}")
    file(WRITE "${_page}" "${_html}")
endforeach()
