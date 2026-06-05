@echo off
chcp 65001 >nul
    echo âŒ Main install failed!
    pause 
    exit /b 
)
echo Running Console Demo...
cd examples\Demo
call mvn compile exec:exec -q
cd ..\..
pause
