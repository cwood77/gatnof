@echo off

if "%1" == "bless" goto bless

bin\out\debug\coml.exe --generate test/screen.test.coml bin/out/debug/screen.test.cpp
if not %ERRORLEVEL% == 0 goto failed

fc bin\out\debug\screen.test.hpp test\expected.hpp > nul
if not %ERRORLEVEL% == 0 goto failed
fc bin\out\debug\screen.test.cpp test\expected.cpp > nul
if not %ERRORLEVEL% == 0 goto failed

echo -) coml test passed
goto end

:bless
copy bin\out\debug\screen.test.hpp test\expected.hpp
copy bin\out\debug\screen.test.cpp test\expected.cpp
goto end

:failed
echo ==) coml test FAILED

:end
