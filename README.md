Helper Class for CERN ROOT Canvas
=================================

<p><img src="https://unpkg.com/ionicons@5.5.2/dist/svg/home-sharp.svg" width="24" height="24"> Project homepage: <a href="https://petrstepanov.com/root-canvas-helper/">https://petrstepanov.com/root-canvas-helper</a></p>

<p><img src="https://unpkg.com/ionicons@5.5.2/dist/svg/logo-github.svg" width="24" height="24" style="vertical-align: middle; margin-right: 0.25rem"> Github repository: <a href="https://github.com/petrstepanov/root-canvas-helper">https://github.com/petrstepanov/root-canvas-helper</a></p>

By default, ROOT graphics on the canvas scales according to the canvas size. The bigger the canvas size is - the larger corresponding fonts, paddings are. This approach works good for canvases that contain one or two sub-pads. However, when ROOT canvas is divided into a higher number of sub-pads, say, 3x3, 5x5 or more, following issues take place:

* Absolute pixel font size on every child pad becomes too small and almost unreadable.
* Font sizes are inconsistent across various canvas elements (no typographic scale).
* Text in the legend and statistic boxed becomes very small.
* By default, ROOT canvas fas a fixed left margin value. Therefore, the Y axis title sometimes gets pushed outside the canvas area.

Above issues are especially notable on screens with smaller resolution. For instance, let us create a test ROOT canvas divided into 4 sub-pads. On the screenshot below, canvas size is set to 800x600 pixels. 

<p align="center">
  <img width="85%" src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-default.png" alt="Standard ROOT canvas with multiple sub pads" />
</p>

It is quite clear that sharing the plots with such a small text is rather unacceptable. Often when sharing screen at the online conferences, participants can hardly see the results.

Indeed, ROOT allows for setting a fixed pixel size font. However, this is usually not enough. A nice looking multi-pad plot also requires tweaking of the number of the tick marks, adjusting frame margins, tick mark length, changing title font sizes, axis title spacing and many more. ROOT is a very flexible framework and majority of these things can be easily tweaked. But at what cost? It comes down to duplicating chunks of code for every particular canvas. Moreover, 

This library is aimed on fixing issues outlined above. Canvas Helper converts ROOT plots from fractional-based geometry to fixed pixel sizes. As a result, the plot looks more alike to what Gnuplot output is.:

<p align="center">
  <img width="85%" src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-processed.png" alt="ROOT canvas processed with CanvasHelper" />
</p>

A single line of code is required to make the change happen. We simply pass the canvas instance to the library after all the primitives were created:
```
CanvasHelper::getInstance()->addCanvas(yourCanvas);
```

List of Fetures
---------------
* Adding a joint title for canvas that was divided into multiple pads. This was frequently asked on the ROOT forum.
* Library also allows adding a subtitle to the ROOT canvas. This may be useful for plots that need some extra information.

<p align="center">
  <img width="85%" src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/multi-pad-canvas-title.png" alt="Add title for ROOT canvas with multiple pads" />
</p>

* Impemented rounding of the parameter values and errors inside the statistics box. Parameter values round to the first significant digit of their errors. This improves visual clarity of the data.

<p align="center">
  <img width="85%" src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/cern-root-parameter-values-rounding.png" alt="Rounding of the parameter values for ROOT statistics box" />
</p>

* Stat boxes and legends can be force aligned to canvas edges. Single edge or a combination of two (e.g. top & left, bottom & right) are supported.

* Another cool feature is that registered canvases and sub-pads automatically re-adjust all the dimensions and their primitives upon the resize event.

<p align="center">
  <img width="85%" src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-resize.png" alt="Resizing a ROOT canvas" />
</p>

Installation with CMake • Preferred
-----------------------------------

This is the preferred installation option. Installation depends on prerequisites:
* Have `cmake` version 3.XX program installed on computer. Usually CMake is included in Linux "Development Tools" group package. On older distributions CMake v3.XX may be named `cmake3`.
* ROOT environment should be sourced in the current shell. Tested with ROOT v6.26.XX.

First we download the repository and create an out-of-source build folder.

```
mkdir -p ~/Downloads && cd ~/Downloads
git clone https://github.com/petrstepanov/root-canvas-helper
mkdir -p ./root-canvas-helper-build && cd ./root-canvas-helper-build
```

Next we invoke CMake. It ensures that all the dependencies are satisfied and generates GNU `Makefile`. Finally, we execute Makefile's `install` target:

```
cmake ../root-canvas-helper
cmake --build . --target install
```

Makefile generates dictionary, builds shared libaray, compiles code into object files, links the executable, and installs corresponding files files in required locations. 

**Tip**. If above command requires administrator privilleges that current user does not have, the install prefix can be changed to a local install:

```
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ../root-canvas-helper
cmake --build . --target install
```

Additionally, `$PATH`, `$LD_LIBRARY_PATH`, and  `$CPLUS_INCLUDE_PATH` environemnt variables need to be modified for the library to be discoverable. Below please find an example how to do it in BASH and CSH shells:

### BASH Shell
```
echo "export PATH=$HOME/.local/bin:$PATH" >> $HOME/.bashrc
echo "export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH" >> $HOME/.bashrc
echo "export CPLUS_INCLUDE_PATH=$HOME/.local/include:$CPLUS_INCLUDE_PATH" >> $HOME/.bashrc
```

### CSH-Based Shells
```
echo "setenv PATH $HOME/.local/bin:$PATH" >> $HOME/.cshrc
echo "setenv LD_LIBRARY_PATH $HOME/.local/lib:$LD_LIBRARY_PATH" >> $HOME/.cshrc
echo "setenv CPLUS_INCLUDE_PATH $HOME/.local/include:$CPLUS_INCLUDE_PATH" >> $HOME/.cshrc
```

This should do the trick.

Manual Installation • Optional
------------------------------

This is an alternative way of building the the library with Cling interpreter. Check out the repository:

```
mkdir -p ~/Downloads && cd ~/Downloads
git clone https://github.com/petrstepanov/root-canvas-helper
cd ./root-canvas-helper/src
```

Make sure your ROOT environment is set up. Next, enter the Cling interpreter shell and compile a shared library from sources:

```
root
.L CanvasHelper.cpp+
.q
```

Now that the shared library is compiled we install the `.so` library, `.pcm` dictionary and `*.h` header files. Commands below may require administrative persissions:

```
cp CanvasHelper*.so CanvasHelper*.pcm CanvasHelper*.d `root-config --libdir`
cp CanvasHelper*.h `root-config --incdir`
cp canvasHelperDemo.cpp $ROOTSYS/macros
```

Now that the library is installed, user should be able to run the demo macros:

```
root canvasHelperDemo.cpp
```

How to Use the Library
----------------------

### In a ROOT Macro
After the library was installed, it needs to be loaded into the interpreter session in your ROOT script:

```
#ifdef __CINT__
  gSystem->Load("CanvasHelper");
#endif
```

### In a ROOT-based Program

If developing a ROOT-based project (not a ROOT macro script), corresponding library header file needs to be included `#include <CanvasHelper.h>`. Program needs to be link against the CanvasHelper shared library. Library should be discoverable witn CMake's `find_library(...)`. function.

Documentation and Code Samples
------------------------------

Please refer to the Doxygen documentation to get familiar with all functionality: 
https://petrstepanov.com/root-canvas-helper/classCanvasHelper.html

Find a demo ROOT macros utilizing the CanvasHepler library here:
https://github.com/petrstepanov/root-canvas-helper/blob/main/src/canvasHelperDemo.cpp

How to Contribute
-----------------

Feel free to contribute or suggest any useful features. Library can be compiled with debug symbols and/or imported to the IDE of your choice. Following CMake variable should be specified to successfully build the library in the IDE:

```
CMAKE_BUILD_TYPE:=Debug
ROOT_DIR=<path-to-root-compiled-with-debug-symbols>/cmake
```

How Add to CMake Project
------------------------

Please refer to this GitHub repository to find an example of integration of this library into a CMake-Based project:

https://github.com/petrstepanov/light-guides/tree/main/draw

Thank you for your attention!
