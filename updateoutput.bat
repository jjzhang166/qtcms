@echo off
set /p updateDir="witch one do you wanto update? 1 for debug; 2 for release; 3 for all."

if %updateDir% == 1 (
set desDir=.\output\Debug\
goto :copyfile
)
if %updateDir% == 2 (
set desDir=.\output\Release\
goto :copyfile
)
if %updateDir% == 3 (goto :all) else (goto :errorend)

:copyfile
xcopy web\*.* %desDir%\skins\default\ /Y/S/E
copy src\libpcom\pcom_config.xml %desDir%
goto :end


:all
xcopy web\*.* .\output\Debug\skins\default\ /Y/S/E
copy src\libpcom\pcom_config.xml .\output\Debug\ /Y/S/E
xcopy web\*.* .\output\Release\skins\default\ /Y/S/E
copy src\libpcom\pcom_config.xml .\output\Release\ /Y/S/E

:end
pause
exit

:errorend
echo Input error
pause
exit