@echo off
echo ⚡ Building and Installing Main Project (FastAudioPlayer)...
call mvn clean install -DskipTests -q
if %ERRORLEVEL% NEQ 0 ( 
    echo ❌ Main install failed!
    pause 
    exit /b 
)
echo 🚀 Running Latency Benchmark...
cd examples\Benchmark
call mvn compile exec:exec -q
cd ..\..
pause
