@echo off
title Compilado de main.c

:: --- Cargar entorno de Visual Studio ---
set "VS_PATH="

if exist "%VS_PATH%" (
    call "%VS_PATH%" x64
) else (
    echo.
    echo [ERROR] No se encontro el entorno de Visual Studio. Asegurate de tenerlo instalado y ajusta la ruta en este script.
    echo.
    pause
    exit /b 1
)
:: ---------------------------------------

:: 1. Intentar borrar el ejecutable anterior para limpiar el área
if exist main.exe del main.exe

@echo Compilando main.c, gs.c y trx.c...
:: 2. Compilar todos los archivos fuente con las librerías necesarias
cl -Tc ../proyecto-final/src/main.c -Tc ../proyecto-final/src/gs.c -Tc ../proyecto-final/src/trx.c ../proyecto-final/src/model.c ../proyecto-final/src/texture.c -I".\include" -link User32.lib Gdi32.lib Opengl32.lib Xinput9_1_0.lib .\lib\Osw.lib -OUT:main.exe

:: 3. Verificar si la compilación fue exitosa
if errorlevel 1 (
    echo.
    echo [ERROR] Hubo un problema al compilar main.c. Revisa el codigo arriba.
    echo.
) else (
    echo -------------------------------------------
    echo Compilacion exitosa. Ejecutando main.exe...
    echo -------------------------------------------
    main.exe
)

:: 4. Pausar para ver los resultados antes de cerrar la ventana
echo.
pause