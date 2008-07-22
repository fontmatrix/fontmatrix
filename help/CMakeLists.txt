## Help ##

# Now it’s just a matter of maintaining a list of all
# needed files, when releasing - pm

SET(FM_HELP_DIR ${CMAKE_SOURCE_DIR}/help/)

SET( FM_HELP_HTML
${FM_HELP_DIR}help_da.html
${FM_HELP_DIR}help_de.html
${FM_HELP_DIR}help_en.html
${FM_HELP_DIR}help_fi.html
${FM_HELP_DIR}help_fr.html
${FM_HELP_DIR}help_hi.html
${FM_HELP_DIR}help_nb.html
${FM_HELP_DIR}help_nl.html
${FM_HELP_DIR}help_no.html
${FM_HELP_DIR}help_ru.html
${FM_HELP_DIR}help_sr.html
${FM_HELP_DIR}help_sv.html
${FM_HELP_DIR}help_uk.html
${FM_HELP_DIR}help_zh_CN.html
${FM_HELP_DIR}help_zh_TW.html
)

SET( FM_HELP_CSS
${FM_HELP_DIR}helpbase.css
)

SET( FM_HELP_JS
${FM_HELP_DIR}js/jquery.js
${FM_HELP_DIR}js/jquery.jtabs.pack.js
)

SET( FM_HELP_GRAPHICS
# DA
# DE
# EN
${FM_HELP_DIR}img/en/fontmatrix_en.png
${FM_HELP_DIR}img/en/fontbook_en.png
${FM_HELP_DIR}img/en/tagset_en.png
${FM_HELP_DIR}img/en/import_en.png
${FM_HELP_DIR}img/en/preferences_en.png
# FI
# FR
# HI
# NB
# NL
# NO
# RU
# SR
# SV
# UK
# ZH CN
# ZH TW
)

SET( FM_HELP_FILES
${FM_HELP_HTML}
${FM_HELP_CSS}
${FM_HELP_JS}
${FM_HELP_GRAPHICS}
)
