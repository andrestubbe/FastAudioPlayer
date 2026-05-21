@echo off
chcp 65001 >nul
echo ⚡ Building and Installing Main Project (FastAudioPlayer)...
call mvn clean install -DskipTests -q
if %ERRORLEVEL% NEQ 0 ( 
    echo ❌ Main install failed!
    pause 
    exit /b 
)
echo Running Console Demo...
cd examples\Demo
call mvn compile exec:exec -q
cd ..\..
pause
