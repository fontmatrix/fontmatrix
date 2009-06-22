SET(FM_HELP_FILES_EN
menu.xml
style.css
index.html
news.html
interface.html
management.html
management_importing.html
management_tags.html
viewing.html
viewing_overview.html
viewing_sample_text.html
viewing_glyphs_chart.html
search.html
search_in_generic_metadata.html
search_in_panose.html
search_using_bitmap_samples.html
comparisons.html
comparisons_fonts.html
comparisons_glyphs.html
output.html
additional.html
additional_db_check.html
additional_editing_panose.html
additional_ttf_tables.html
additional_extracting_fonts.html
additional_export_for_packaging.html
scripting.html
resources.html
export_fonts.html
printing_fonts.html
font_books.html
creating_font_books.html
font_book_templates.html
setting_up.html
setting_up_preferences.html
setting_up_preferences_general.html
setting_up_preferences_system_tray.html
setting_up_preferences_display.html
setting_up_preferences_service.html
setting_up_preferences_samples_collection.html
setting_up_preferences_files_and_folders.html
setting_up_preferences_shortcuts.html
setting_up_layout_engine_options.html
)

FILE( GLOB FM_HELP_IMAGES_EN images/*.png )


INSTALL(FILES ${FM_HELP_FILES_EN}
	DESTINATION "${HELPDIR}/en")

INSTALL(FILES ${FM_HELP_IMAGES_EN}
	DESTINATION "${HELPDIR}/en/images")
	
