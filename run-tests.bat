@echo off
setlocal enabledelayedexpansion

REM 1. Set the root directory you want to search
set "rootSource=C:\Users\amoura\Code\amc\tests"

echo Russing tests in: %rootSource%
echo --------------------------------------------------

REM 2. Loop through all files recursively
for /r "%rootSource%" %%f in (*) do (
    
    REM Get the name of the parent folder of the current file
    set "parentDir=%%~dpf"
    
    REM 3. Logic: Check the folder name to decide the command
    REM We use "findstr" to see if the path contains a specific keyword
    
    echo "%%~dpf" | findstr /i "invalid" >nul
    if !errorlevel! == 0 (
        REM Run your 'Convert' command here
        REM your-convert-command "%%f" >nul 2>&1
        echo invalid: %%~dpf%%~nxf
    ) else (
        echo "%%~dpf" | findstr /i "valid" >nul
        if !errorlevel! == 0 (
            REM Run your 'Analyze' command here
            REM your-analyze-command "%%f" >nul 2>&1
            echo valid: %%~dpf%%~nxf
        )
    )

    REM 4. Capture the return code of the execution
    set "lastCode=!errorlevel!"

    REM 5. Report if the return code is non-zero
    if !lastCode! neq 0 (
        echo [FAILURE]
        echo "Location: %%~dpf"
        echo "File:     %%~nxf"
        echo --------------------------------------------------
    )
)

echo Tests complete.
