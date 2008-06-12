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

IF ( IS_TRANSLATOR )
	QT4_CREATE_TRANSLATION ( fontmatrix_QMS ${fontmatrix_SRCS} ${fontmatrix_UIS_H} ${fontmatrix_TSS} )
ELSE( IS_TRANSLATOR )
	QT4_ADD_TRANSLATION ( fontmatrix_QMS ${fontmatrix_TSS} )
ENDIF( IS_TRANSLATOR )

ADD_CUSTOM_TARGET ( translation
                    ALL
                    DEPENDS  ${fontmatrix_QMS}
                  )

SET ( QMDIR "${CMAKE_INSTALL_PREFIX}/share/fontmatrix/qm/" )
INSTALL ( FILES
          ${fontmatrix_QMS}
          DESTINATION ${QMDIR}
        )
