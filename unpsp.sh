#!/bin/sh

PSPVOL=/Volumes/Untitled\ 1

# echo $PSPVOL

if [ ! -d "$PSPVOL"/. ]
then
	echo "PSP is not connected"
	exit
fi

diskutil unmount "$PSPVOL"
