@echo off
rd /s /q project_files
mkdir project_files
cd project_files
cmake -G "Visual Studio 14 2015" ../source/
cd ..