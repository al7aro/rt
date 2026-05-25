@echo off
cd /d "%~dp0"
cd ..
set VENDOR_DIR=vendor
if not exist %VENDOR_DIR% mkdir %VENDOR_DIR%

@REM FETCH GLFW
set REPO_URL=https://github.com/glfw/glfw.git
git clone %REPO_URL% -b 3.3-stable %VENDOR_DIR%\glfw

@REM FETCH GLAD
set REPO_URL=git@github.com:al7aro/glad.git
git clone %REPO_URL% %VENDOR_DIR%\glad

@REM FETCH GLM
set REPO_URL=https://github.com/g-truc/glm.git
git clone %REPO_URL% -b 1.0 %VENDOR_DIR%\glm

@REM FETCH STB
set REPO_URL=git@github.com:al7aro/stb.git
git clone %REPO_URL% %VENDOR_DIR%\stb

@REM FETCH IMGUI
set REPO_URL=git@github.com:al7aro/imgui.git
git clone %REPO_URL% %VENDOR_DIR%\imgui

pause