SET ( fontmatrix_TSS
      fontmatrix-da.ts
      fontmatrix-de.ts
      fontmatrix-en.ts
      fontmatrix-fi.ts
      fontmatrix-fr.ts
      fontmatrix-hi.ts
      fontmatrix-nb.ts
      fontmatrix-nl.ts
      fontmatrix-no.ts
      fontmatrix-ru.ts
      fontmatrix-sr.ts
      fontmatrix-sv.ts
      fontmatrix-uk.ts
      fontmatrix-zh_CN.ts
      fontmatrix-zh_TW.ts
    )

IF ( TRANSLATOR )
	MESSAGE(STATUS "We are glad to see you handle the fontmatrix-${TRANSLATOR}.ts file, thank you very much")
	QT4_CREATE_TRANSLATION ( fontmatrix_QMS ${fontmatrix_SRCS} ${fontmatrix_UIS} "fontmatrix-${TRANSLATOR}.ts")
	QT4_ADD_TRANSLATION ( fontmatrix_QMS ${fontmatrix_TSS} )
ELSE( TRANSLATOR )
	QT4_ADD_TRANSLATION ( fontmatrix_QMS ${fontmatrix_TSS} )
ENDIF( TRANSLATOR )

ADD_CUSTOM_TARGET ( translation ALL DEPENDS ${fontmatrix_QMS} )

IF(UNIX AND NOT APPLE)
	INSTALL ( FILES ${fontmatrix_QMS} DESTINATION ${FONTMATRIX_QMDIR} )
ENDIF(UNIX AND NOT APPLE)

IF(APPLE)
# 	ADD_EXECUTABLE ( fontmatrix MACOSX_BUNDLE ${fontmatrix_QMS} )
	SET_SOURCE_FILES_PROPERTIES( ${fontmatrix_QMS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
# 	INSTALL ( TARGETS fontmatrix BUNDLE DESTINATION  ${FONTMATRIX_BUNDLE_RESOURCE})
ENDIF(APPLE)

IF(WIN32)
	INSTALL ( FILES ${fontmatrix_QMS} DESTINATION ${FONTMATRIX_QMDIR} )
ENDIF(WIN32)

