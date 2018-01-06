@echo off
cd /d %~dp0
doxygen souidoc
rd /s/q "doc/html"