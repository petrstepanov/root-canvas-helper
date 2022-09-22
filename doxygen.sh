#!/bin/bash

mkdir -p ./docs/xml
doxygen
moxygen ./docs/xml
