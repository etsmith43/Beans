@echo off
cls

set EXE_NAME=beans

g++ -std=c++14 %EXE_NAME%.cpp -luser32 -lgdi32 -lopengl32 -lgdiplus -o %EXE_NAME%.exe
%EXE_NAME%