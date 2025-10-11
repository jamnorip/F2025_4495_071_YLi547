@echo off
set HOST_IP=140.161.83.22

cd /d "C:\Users\300380547\Desktop\Windows\Windows\WorldRebalance\Binaries\Win64"
echo Starting Unreal Client and connecting to %HOST_IP%:7777 ...
WorldRebalance.exe -log -WINDOWED -ResX=1280 -ResY=720 -ExecCmds="open %HOST_IP%"
pause
