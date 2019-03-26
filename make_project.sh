#!/bin/bash
rm -rf project_files
mkdir project_files
cd project_files
cmake -G Xcode ../source/
