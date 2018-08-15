SET PATH=%PATH%;%~dp0\..\..\dep\msvc\qt5-64\bin

moc -o moc_debuggerwindow.cpp debuggerwindow.h
moc -o moc_displaywindow.cpp displaywindow.h
moc -o moc_emuthread.cpp emuthread.h
moc -o moc_mainwindow.cpp mainwindow.h

uic -o ui_debuggerwindow.h debuggerwindow.ui
uic -o ui_mainwindow.h mainwindow.ui
rcc -o rcc_icons.cpp resources/icons.qrc

pause