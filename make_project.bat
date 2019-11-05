@echo off
mkdir project_files
cd project_files

:: Check visual studios 2017, 2015 in order and generate with the first one found
reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.15.0" >> nul 2>&1
if %ERRORLEVEL% NEQ 0 ( echo Visual Studio 2017 not installed ) else (
	echo Generating project for Visual Studio 2017
	cmake -G "Visual Studio 15 2017" ../source/ 
	cd ..
	exit /b
)

reg query "HKEY_CLASSES_ROOT\VisualStudio.DTE.14.0" >> nul 2>&1
if %ERRORLEVEL% NEQ 0 ( echo Visual Studio 2015 not installed ) else (
	echo Generating project for Visual Studio 2015
	cmake -G "Visual Studio 14 2015" ../source/ 
	cd ..
	exit /b
)