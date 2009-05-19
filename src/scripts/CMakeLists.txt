# Scripts CMakeLists

SET(SAMPLE_SCRIPTS
fontforge_subset.py
fonttools_showxml.py
xetex_fontbook.py
import_tags.py
)

IF(UNIX AND NOT APPLE)
	SET(SAMPLE_SCRIPTS_DIR "share/fontmatrix/resources/Scripts/")
	
	INSTALL(FILES
	${SAMPLE_SCRIPTS}
	DESTINATION "${SAMPLE_SCRIPTS_DIR}"
	)
	
ENDIF(UNIX AND NOT APPLE)