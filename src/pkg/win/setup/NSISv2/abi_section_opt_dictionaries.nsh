;Title          AbiWord for Windows, NSIS v2 series installer script
;FileDesc       Contains optionally included sections for dictionary (hash) files


SubSection /e "$(TITLE_ssection_dictionary)" ssection_dictionary

; OPTIONAL Installation of Default Dictionary
Section "$(TITLE_section_dictinary_def_English)" section_dictinary_def_English
	SectionIn 1 2 ${DLSECT}
	SetOutPath $INSTDIR
	File /r "..\AbiSuite\dictionary"
SectionEnd
!macro Remove_${section_dictinary_def_English}
	;Removes this component
	DetailPrint "*** Removing or skipping install of en-US dictionary..."

	; remove dictionary
	Delete "$INSTDIR\dictionary\american.hash"
!macroend


!ifdef OPT_DICTIONARIES
!include "abi_dict_misc.nsh"
!endif ; OPT_DICTIONARIES


SubSectionEnd ; Dictionaries
!macro Remove_${ssection_dictionary}
	; Note: subsection removes called unless every section contained is selected
	;       so do not actually remove anything that may be necessary
	;       if subsection is only partially selected
	DetailPrint "*** ssection_dictionary"

	; remove ispell dictionary info file
	${DoIfDirLacks} "*.hash"  "$INSTDIR\dictionary" "Delete 'ispell_dictionary_list.xml'"

	; attempt to remove dictionary directory, if no more are present
	${DeleteDirIfEmpty} "$INSTDIR\dictionary"
!macroend
