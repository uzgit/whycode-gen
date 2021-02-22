#!/bin/bash

SVGs=*.svg

for SVG in $SVGs
do
	PNG="${SVG//svg/png}"
	echo "Converting $SVG to $PNG..."
	convert $SVG $PNG
done

echo "Removing .svg files..."
rm $SVGs
echo "Done!"
