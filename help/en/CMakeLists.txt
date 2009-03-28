SET(FM_HELP_FILES_EN
menu.xml
style.css
index.html
news.html
interface.html
management.html
search.html
comparisons.html
output.html
additional.html
additional_db_check.html
additional_editing_panose.html
additional_ttf_tables.html
scripting.html
resources.html
export_fonts.html
printing_fonts.html
font_books.html
creating_font_books.html
font_book_templates.html
setting_up.html
setting_up_preferences_general.html
setting_up_preferences_system_tray.html
setting_up_preferences_display.html
setting_up_preferences_service.html
setting_up_preferences_samples_collection.html
setting_up_preferences_files_and_folders.html
setting_up_preferences_shortcuts.html
setting_up_layout_engine_options.html
)

INSTALL(FILES ${FM_HELP_FILES_EN}
	DESTINATION "${HELPDIR}/en")
