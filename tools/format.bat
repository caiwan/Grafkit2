@echo off

set CLANG_TIDY_EXE="clang-tidy"
set CLANG_FORMAT_EXE="clang-format"
set SOURCE_DIR=%cd%/src
set HEADER_DIR=%cd%/include

set CLANG_TIDY_COMMAND=%CLANG_TIDY_EXE% --config-file=.clang-tidy --fix --fix-errors
set CLANG_FORMAT_COMMAND=%CLANG_FORMAT_EXE% --style=file -i "%%f"

rem

echo Running clang-tidy on all source files...
for /r %HEADER_DIR% %%f in (*.cpp, *.h) do (
  %CLANG_TIDY_COMMAND% "%%f"
)

for /r %SOURCE_DIR% %%f in (*.cpp, *.h) do (
  %CLANG_TIDY_COMMAND% "%%f"
)

rem

echo Running clang-format on all source files...

for /r %SOURCE_DIR% %%f in (*.cpp, *.h) do (
  %CLANG_FORMAT_COMMAND% "%%f"
)

for /r %SOURCE_DIR% %%f in (*.cpp, *.h) do (
  %CLANG_FORMAT_COMMAND% "%%f"
)

echo Done.
