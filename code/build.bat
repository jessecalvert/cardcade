@REM @Author: Jesse
@REM @Date:   2016-08-26 15:46:32
@REM @Last Modified by:   Jesse
@REM Modified time: 2017-07-16 22:50:29

@echo off

set CommonCompilerFlags=-Od -MTd -nologo -Gm- -GR- -EHa- -Oi -WX -W4 -wd4127 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7 /F 0x1000000
set CommonCompilerFlags=-DGAME_DEBUG=1 -DGAME_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib ws2_32.lib

IF NOT EXIST W:\cardcade\build mkdir W:\cardcade\build
pushd W:\cardcade\build
W:\ctime\ctime -begin cardcade.ctm
del *.pdb > NUL 2> NUL

cl %CommonCompilerFlags% -wd4456 W:\cardcade\code\asset_builder.cpp /link %CommonLinkerFlags%
rem cl %CommonCompilerFlags% -wd4456 W:\cardcade\code\simple_meta.cpp /link %CommonLinkerFlags%
rem cl %CommonCompilerFlags% -wd4456 W:\cardcade\code\win32_cardcade_server.cpp /link %CommonLinkerFlags%
rem cl %CommonCompilerFlags% -wd4456 W:\cardcade\code\render_test.cpp /link %CommonLinkerFlags%
cl %CommonCompilerFlags% -wd4456 W:\cardcade\code\slaedit.cpp /link %CommonLinkerFlags%

w:\cardcade\build\simple_meta.exe w:\cardcade\code 1 cardcade.cpp generated.h

rem rem 64-bit build
rem rem Optimization switches /O2
echo WAITING FOR PDB > lock.tmp

cl %CommonCompilerFlags% W:\cardcade\code\cardcade.cpp -Fmcardcade.map -LD /link -incremental:no -opt:ref -PDB:cardcade_%random%.pdb -EXPORT:GameUpdateAndRender -EXPORT:DebugFrameEnd
cl %CommonCompilerFlags% W:\cardcade\code\win32_cardcade_opengl.cpp -Fmwin32_cardcade_opengl.map -LD /link %CommonLinkerFlags% -PDB:cardcade_renderer%random%.pdb -EXPORT:Win32LoadOpenGLRenderer

del lock.tmp

cl %CommonCompilerFlags% W:\cardcade\code\win32_cardcade.cpp -Fmwin32_cardcade.map /link %CommonLinkerFlags%

W:\ctime\ctime -end cardcade.ctm
popd
