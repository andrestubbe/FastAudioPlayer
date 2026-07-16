@echo off
    echo âŒ Main install failed!
    pause 
    exit /b 
)
echo Running Console Demo...
cd examples\Demo
call mvn compile exec:exec
cd ..\..
pause
