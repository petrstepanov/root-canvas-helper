Helper Class for CERN ROOT Canvas
=================================

TODO: what it does?

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
cp *.so `root-config --libdir`
cp *.pcm `root-config --libdir`
cp *.h `root-config --incdir`
```

This is it. Now you can utilize the library. Feel free to inspect and run an example ROOT macro `test.c` that demonstrates the use of the library:
```
root test.c
```

Integrate into your ROOT script
-------------------------------


Add to a ROOT-based Project
---------------------------
