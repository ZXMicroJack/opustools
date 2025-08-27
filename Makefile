
targets=mkdisk extractopd mkdisk.exe extractopd.exe

default: ${targets}

rel: opustools.zip

mkdisk: mkdisk.c
	gcc -o$@ $<

extractopd: extractopd.c
	gcc -o$@ $<

mkdisk.exe: mkdisk.c
	x86_64-w64-mingw32-gcc -o$@ $<

extractopd.exe: extractopd.c
	x86_64-w64-mingw32-gcc -o$@ $<

opustools.zip: ${targets}
	zip $@ $^

clean:
	rm ${targets}



