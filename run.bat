@echo off
REM 3D数据可视化系统启动脚本（精简版，无自动提示与暂停）

REM 设置Qt DLL路径
set PATH=D:\AppData\QT\6.7.2\mingw_64\bin;D:\AppData\QT\Tools\mingw1120_64\bin;%PATH%

REM 运行程序
cd /d "%~dp0build"
start "" 3DDataVisualization.exe
