@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
@REM
@REM -- Intel Copyright Notice -- 
@REM  
@REM @par 
@REM Copyright (c) 2002-2011 Intel Corporation All Rights Reserved. 
@REM  
@REM @par 
@REM The source code contained or described herein and all documents 
@REM related to the source code ("Material") are owned by Intel Corporation 
@REM or its suppliers or licensors.  Title to the Material remains with 
@REM Intel Corporation or its suppliers and licensors. 
@REM  
@REM @par 
@REM The Material is protected by worldwide copyright and trade secret laws 
@REM and treaty provisions. No part of the Material may be used, copied, 
@REM reproduced, modified, published, uploaded, posted, transmitted, 
@REM distributed, or disclosed in any way except in accordance with the 
@REM applicable license agreement . 
@REM  
@REM @par 
@REM No license under any patent, copyright, trade secret or other 
@REM intellectual property right is granted to or conferred upon you by 
@REM disclosure or delivery of the Materials, either expressly, by 
@REM implication, inducement, estoppel, except in accordance with the 
@REM applicable license agreement. 
@REM  
@REM @par 
@REM Unless otherwise agreed by Intel in writing, you may not remove or 
@REM alter this notice or any other notice embedded in Materials by Intel 
@REM or Intel's suppliers or licensors in any way. 
@REM  
@REM @par 
@REM For further details, please see the file README.TXT distributed with 
@REM this software. 
@REM  
@REM @par 
@REM -- End Intel Copyright Notice -- 
@echo off
echo CE7 OS Loader source code ...

del .\MdeModulePkg\CE7OSLoader.*
rmdir /s /q .\MdeModulePkg\Application\CE7OSloader

echo Removing Build and Conf directories ...
if exist Build rmdir Build /s /q
if exist Conf  rmdir Conf  /s /q
if exist Report.log  del Report.log /q /f

