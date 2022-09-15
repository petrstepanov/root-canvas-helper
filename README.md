Helper Class for CERN ROOT Canvas
=================================

By default, ROOT graphics on the canvas scales according to the canvas size. The bigger the canvas size is - the larger corresponding fonts, paddings are. This approach works fine for canvases that contain one or just a few sub-pads. However, when ROOT canvas is divided into a higher number of sub-pads, say, 3x3, 5x5 or more, following issues take place:

* Absolute pixel font size on every child pad becomes too small and almost unreadable.
* Font sizes become inconsistent across canvases with various number of sub-pads.
* Text in the legend and statistic boxed becomes very small.
* By default, ROOT canvas fas a fixed left margin value. Therefore, the Y axis title sometimes gets pushed outside the canvas area.

Above issues are especially notable on screens with smaller resolution. Please refer to an example below. We create a ROOT multi-canvas with 4 sub-pads. Canvas size is set to 800x600 pixels. 

<figure>
  <img src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-default.png" alt="Standard ROOT canvas with multiple sub pads" />
</figure>

It is quite clear that sharing the plots with such a small text is rather unacceptable. Often when sharing screen at the online conferences, participants can hardly see the results.

Indeed, ROOT allows for setting a fixed pixel size font. However, this is usually not enough. A nice looking plot requires also tweaking the number of the tick marks, adjusting frame margins, tick mark length, changing title font sizes, axis title spacing and many more. Yes, technically everything can be tweaked. But at what cost? It comes down to duplicating chunks of code for every canvas.

This library is aimed on fixing issues outlined above. It is enough to pass the instance of a canvas to the library:
```
CanvasHelper::getInstance()->addCanvas(yourCanvas);
```

Canvas Helper transforms original canvas in a following way:

<figure>
  <img src="https://raw.githubusercontent.com/petrstepanov/root-canvas-helper/main/resources/canvas-processed.png" alt="ROOT canvas processed with CanvasHelper" />
</figure>

Additionally, this library provides a few useful features, such as:
* Impemented proper rounding of the parameter values and errors in the statistics box. Parameter values always round to the first significant digit of the error.
* Alignment of the stat box or legend to the canvas frame corners top-left, bottom-right etc...
* Adding a joint title for canvas that was divided into multiple pads.

Another cool feature is that registered canvases and sub-pads automatically re-adjust all the dimensions and their primitives upon the resize event.

How to Install
--------------

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

This is it. Now you can utilize the library. Feel free to inspect and run an example ROOT macro `test.c` that demonstrates the use of the library:
```
root demo.cpp
```

Integrate into your ROOT script
-------------------------------


Add to a ROOT-based Project
---------------------------
