Helper Class for CERN ROOT Canvas
=================================

By default, ROOT graphics on the canvas scales according to the canvas size. The bigger the canvas size is - the larger corresponding fonts, paddings are. This approach works good for canvases that contain one or two sub-pads. However, when ROOT canvas is divided into a higher number of sub-pads, say, 3x3, 5x5 or more, following issues take place:

* Absolute pixel font size on every child pad becomes too small and almost unreadable.
* Font sizes are inconsistent across various canvas elements (no typographic scale).
* Text in the legend and statistic boxed becomes very small.
* By default, ROOT canvas fas a fixed left margin value. Therefore, the Y axis title sometimes gets pushed outside the canvas area.

Above issues are especially notable on screens with smaller resolution. For instance, let us create a test ROOT canvas divided into 4 sub-pads. On the screenshot below, canvas size is set to 800x600 pixels. 

<figure>
  <img src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-default.png" alt="Standard ROOT canvas with multiple sub pads" />
</figure>

It is quite clear that sharing the plots with such a small text is rather unacceptable. Often when sharing screen at the online conferences, participants can hardly see the results.

Indeed, ROOT allows for setting a fixed pixel size font. However, this is usually not enough. A nice looking multi-pad plot also requires tweaking of the number of the tick marks, adjusting frame margins, tick mark length, changing title font sizes, axis title spacing and many more. ROOT is a very flexible framework and majority of these things can be easily tweaked. But at what cost? It comes down to duplicating chunks of code for every particular canvas. Moreover, 

This library is aimed on fixing issues outlined above. Canvas Helper converts ROOT plots from fractional-based geometry to fixed pixel sizes. As a result, the plot looks more alike to what Gnuplot output is.:

<figure>
  <img src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-processed.png" alt="ROOT canvas processed with CanvasHelper" />
</figure>

A single line of code is required to make the change happen. We simply pass the canvas instance to the library:
```
CanvasHelper::getInstance()->addCanvas(yourCanvas);
```

Additionally, this library provides a few useful features, such as:
* Impemented proper rounding of the parameter values and errors in the statistics box. Parameter values always round to the first significant digit of the error.
* Alignment of the stat box or legend to the canvas frame corners top-left, bottom-right etc...
* Library also allows adding a subtitle to the ROOT canvas. This may be useful for plots that need some elaboration.
* Adding a joint title for canvas that was divided into multiple pads. This was frequently asked on the ROOT forum.

Another cool feature is that registered canvases and sub-pads automatically re-adjust all the dimensions and their primitives upon the resize event.

How to Use with ROOT Scripts
----------------------------

To make use of this helper class, check out the repository:
```
mkdir -p ~/Downloads
cd ~/Downloads
git clone https://github.com/petrstepanov/root-canvas-helper
cd .root-canvas-helper
```

Next, enter the Cling interpreter shell and compile a shared library from sources:
```
root
.L CanvasHelper.cpp+
.q
```

Now that the shared library is compiled we install the `.so` library, `.pcm` dictionary and `*.h` header files. Commands below may require root persissions `sudo`:
```
cp CanvasHelper*.so `root-config --libdir`
cp CanvasHelper*.pcm `root-config --libdir`
cp CanvasHelper*.h `root-config --incdir`
```

This is it. Now you can utilize the library. Feel free to inspect and run an example ROOT macro `demo.cpp` that demonstrates a few use cases:
```
root demo.cpp
```

After the library was installed, it needs to be loaded into the interpreter session in your ROOT script:
```
gSystem->Load("CanvasHelper_cpp.so");
```

### Adding a multi-pad title

### Adding a canvas subtitle

### Aligning the statbox or legend


Add library to a CMake-Based Project
------------------------------------

Add to a Makefile-Based Project
-------------------------------