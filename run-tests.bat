@echo off
setlocal enabledelayedexpansion

REM 1. Set the root directory you want to search
set "rootSource=C:\Users\amoura\Code\amc\tests"

echo Running tests in: %rootSource%
echo --------------------------------------------------

REM 2. Loop through all files recursively
for /r "%rootSource%" %%f in (*) do (
    
    REM Get the name of the parent folder of the current file
    set "parentDir=%%~dpf"
    
    REM 3. Logic: Check the folder name to decide the command
    REM We use "findstr" to see if the path contains a specific keyword

    set lastCode=0
    
    echo "%%~dpf" | findstr /i "invalid" >nul
    if !errorlevel! == 0 (
        echo "%%~dpf" | findstr /i "parse" >nul
        if !errorlevel! == 0 (
            REM your-convert-command "%%f" >nul 2>&1
            build\amcc "%%f" --parser-test
        ) else (
            echo "%%~dpf" | findstr /i "lex" >nul
            if !errorlevel! == 0 (
                build\amcc "%%f" --lexer-test
            )
        )
        if !errorlevel! == 0 (
            set lastCode=1
        ) else (
            set lastCode=0
        )
    ) else (
        echo "%%~dpf" | findstr /i "valid" >nul
        if !errorlevel! == 0 (
            build\amcc "%%f" --parser-test
            set lastCode=!errorlevel!
        )
    )

    if !lastCode! neq 0 (
        echo [FAILURE]
        echo "Location: %%~dpf"
        echo "File:     %%~nxf"
        echo --------------------------------------------------
    )
)

echo Tests complete.
