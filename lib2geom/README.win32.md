# win32 instructions
## Installation
- Download the latest mingw, gsl, and gtk bundles from [http://inkscape.modevia.com/win32libs/?C=M;O=D](http://inkscape.modevia.com/win32libs/?C=M;O=D)
- Also download Boost from SourceForge and unpack it in a directory of your choice (you specify it's path later on)
- Obtain, compile and install Inkscape svn following these instructions http://inkscape.org/win32/win32buildnotes.html
- unpackage the gsl bundle into the directory with the gtk bundle
- download cmake; install at `C:\cmake`
- check the directories in mingwenv.bat. Are they set correctly for you? (ignore RAGEL_BIN)
- make a build directory
- open cmd.exe
    ```
    mingwenv.bat
    cmake -G "MinGW Makefiles" c:\path\to\2geom # be sure you have a CAPITAL '-G' there!
    cmake -i # especially mind to set the install dir to where you want it installed (probably the gtk folder, e.g. c:/gtk210))
    mingw32-make
    ```
- copy the resulting `.exe` files into the inkscape install directory

If you want to be able to compile the ragel defined svg-parser as well,
you must download ragel from [here](http://www.cs.queensu.ca/~thurston/ragel/)
Unpack the windows binary package into a dir of your own choice.
Check whether ragel's dir is set correctly in mingwenv.bat (RAGEL_BIN).
Make sure you have ragel.exe and rlcodegen.exe in the dir you specify.
If your package does not contain rlcodegen.exe you can copy rlgen-cd.exe
and rename it to rlcodegen.exe :-)

## PYTHON BINDINGS
To install python bindings, you need python2 and cython(>=0.16). Get cython from
[http://www.lfd.uci.edu/~gohlke/pythonlibs/#cython](http://www.lfd.uci.edu/~gohlke/pythonlibs/#cython)
and install it to `$PYTHON_PATH\Libs\site-packages`
32-bit version is your safest bet.
cmake should be able to find cython now. Make sure that `PYTHON_PATH` is set
correctly to python2 in mingwenv.bat.

Continue with installation as described above. When running "cmake -i",
remember to choose to build cython bindings and also lib2geom as a
shared library!

If you get errors about missing DLL file, try copying lib2geom.dll to
cython-bindings directory.

Note: If you have both python2 and python3 installed, make sure cmake uses
python2. To ensure this, choose "advanced options" when running "cmake -i",
scroll and set PYTHON_INCLUDE_DIR and PYTHON_LIBRARY accordingly.

## TROUBLESHOOTING
If things don't work out, here is a list of things you can try.
Type `make --version`. It should display:
```
E:\inkscapelpe>mingw32-make --version
GNU Make 3.80
Copyright (C) 2002  Free Software Foundation, Inc.
```
Or something very similar. If it does not, probably windows finds a different
make (Borland's or another?). Type `set path=`, then start again with
`mingwenv.bat` etc...
