echo Copying mediastreamer2 libs
copy ..\mediastreamer2-antisip-4.3.3-rc2\oRTP\build\win32native\$(ConfigurationName)\*.dll $(TargetDir)
copy ..\mediastreamer2-antisip-4.3.3-rc2\mediastreamer2\build\win32native\$(ConfigurationName)\*.dll $(TargetDir)
copy ..\mediastreamer2-antisip-4.3.3-rc2\win32-bin\lib\*.dll $(TargetDir)

echo Copying unimrcp libs
copy ..\unimrcp-deps\libs\sofia-sip\win32\pthread\*.dll $(TargetDir)
copy ..\unimrcp-deps\libs\apr\$(ConfigurationName)\*.dll $(TargetDir)
copy ..\unimrcp-deps\libs\apr-util\$(ConfigurationName)\*.dll $(TargetDir)
copy ..\unimrcp-deps\libs\sofia-sip\win32\libsofia-sip-ua\$(ConfigurationName)\*.dll $(TargetDir)
copy ..\unimrcp-deps\libs\apr-util\Debug\$(ConfigurationName)\*.dll $(TargetDir)

xcopy  unimrcp  "$(TargetDir)\unimrcp" /E /Y /I

echo Copying configuration file 
copy *.json $(TargetDir)

