
targets=mkdisk extractopd mkdisk.exe extractopd.exe data2tap data2tap.exe tap2data tap2data.exe nlcomp nlcomp.exe nldecomp nldecomp.exe

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

data2tap.exe: data2tap.c
	x86_64-w64-mingw32-gcc -o$@ $<

data2tap: data2tap.c
	gcc -o$@ $<

tap2data: tap2data.c
	gcc -o$@ $<

tap2data.exe: tap2data.c
	x86_64-w64-mingw32-gcc -o$@ $<

nlcomp: nlcomp.c
	gcc -o$@ $<

nlcomp.exe: nlcomp.c
	x86_64-w64-mingw32-gcc -o$@ $<

nldecomp: nldecomp.c
	gcc -o$@ $<

nldecomp.exe: nldecomp.c
	x86_64-w64-mingw32-gcc -o$@ $<

clean:
	rm ${targets}



