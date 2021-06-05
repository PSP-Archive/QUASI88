#/!bin/sh

EBDIR=`ls -d __SCE*`
PSPDIR=/Volumes/Untitled\ 1/PSP/GAME

echo "Copydir : $EBDIR"
echo "PSPDIR : $PSPDIR/%$EBDIR"

if [ ! -d "$EBDIR" ]
then
	echo "EBOOT.PBP is not found"
	exit
fi

if [ ! -d "$PSPDIR" ]
then
	echo "PSP directory is not found"
	exit
fi


if [ ! -d "$PSPDIR/%$EBDIR" ]
then
	mkdir "$PSPDIR/%$EBDIR"
	mkdir "$PSPDIR/$EBDIR"
fi

 
if [ -d "$PSPDIR" ]
then
	echo "Copying.."
	if [ ! -d "$PSPDIR/%$EBDIR" ]
	then
		echo "Making $EBDIR...";
	fi

#	rm -rf "$PSPDIR/%$EBDIR"
#	rm -rf "$PSPDIR/$EBDIR"
	cp -v $EBDIR/* "$PSPDIR/$EBDIR"
	cp -v %$EBDIR/* "$PSPDIR/%$EBDIR"
	cp -nv resource/* "$PSPDIR/$EBDIR"
else
	echo "PSP is not connected"
	exit
fi
