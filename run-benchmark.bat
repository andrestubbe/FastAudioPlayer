@echo off
chcp 65001 >nul
    echo âŒ Main install failed!
    pause 
    exit /b 
)
echo ðŸš€ Running Latency Benchmark...
cd examples\Benchmark
call mvn compile exec:exec -q
cd ..\..
pause
