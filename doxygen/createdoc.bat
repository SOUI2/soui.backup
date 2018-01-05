cd /d %~dp0
doxygen souidoc
hhc\hhc doc/html/index.hhp
rd /s/q "doc/html"