!if 0
Copyright (c) Microsoft Corporation.  All rights reserved.
!endif
!if 0
Use of this sample source code is subject to the terms of the Microsoft
license agreement under which you licensed this sample source code. If
you did not accept the terms of the license agreement, you are not
authorized to use this sample source code. For the terms of the license,
please see the license agreement between you and Microsoft or, if applicable,
see the LICENSE.RTF on your install media or the root of your tools installation.
THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
!endif

wceldr:
!IF "$(_WINCEOSVER)"=="600"
   romimage $(ROMIMAGE_FLAGS) xldr6.bib
   set _RELEASEDIR=$(_WINCEROOT)\platform\$(_TGTPLAT)\target\$(_TGTCPU)\$(WINCEDEBUG)
!ELSE
    romimage $(ROMIMAGE_FLAGS) xldr.bib
!ENDIF
    if exist $(_RELEASEDIR)\bldrS.bin copy /b $(_RELEASEDIR)\xldrS.nb0 + ..\bldrCfg.nb0 + $(_RELEASEDIR)\bldrS.bin $(_RELEASEDIR)\wceldrS
!IF "$(WINCEREL)"=="1"
    del $(_FLATRELEASEDIR)\wceldrS
    if exist $(_RELEASEDIR)\wceldrS xcopy /Q /I $(_RELEASEDIR)\wceldrS $(_FLATRELEASEDIR)
!ENDIF
