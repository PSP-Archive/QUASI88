
SRCFILES = QUASI88PX.PNG document src ptk 88build.mak Makefile.psp makefile \
 unpsp.sh copy2psp.sh q88px.txt


all : make_pbp send

make_pbp :
	make -f makefile.psp
	make -f makefile.psp SCEkxploit

send:
	sh copy2psp.sh
	sh unpsp.sh


tarball :
	tar czvf q88pxsrc`date +"%y%m%d"`.tgz $(SRCFILES)

bindist :
	zip -r q88px`date +"%y%m%d"`.zip EBOOT.PBP q88px.txt

clean :
	make -f makefile.psp clean

clean_conf :
	rm -f ptk/ptk_config.o

