# Copyright 2020 oscillo
# 
# Permission is hereby granted, free of charge,
# to any person obtaining a copy of this software
# and associated documentation files(the "Software"),
# to deal in the Software without restriction,
# including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense,
# and / or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so,
# subject to the following conditions :
# 
# The above copyright notice and this permission
# notice shall be included in all copies or substantial
# portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY
# OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
# LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

all: Kanade vlcwrapper
vlcwrapper: vlcwrapper.c
	gcc -g vlcwrapper.c -o vlcwrapper -lvlc

Kanade: main.o file.o user.o sqlite3.o book.o favorite.o history.o vlc.o player.o omxplayer.o
	gcc `pkg-config --libs gtk+-3.0` main.o file.o user.o book.o favorite.o history.o sqlite3.o omxplayer.o vlc.o player.o -ldl -lpthread -lavformat -lavutil -lavcodec -lvlc -lsystemd -lm -o Kanade

main.o: main.c
	gcc -c -g main.c `pkg-config --cflags gtk+-3.0`

file.o: file.c
	gcc -c -g file.c

user.o: user.c
	gcc -c -g user.c

book.o: book.c
	gcc -c -g book.c

favorite.o: favorite.c
	gcc -c -g favorite.c

history.o: history.c
	gcc -c -g history.c

sqlite3.o: sqlite3.c
	gcc -c sqlite3.c

vlc.o: vlc.c
	gcc -c -g vlc.c

player.o: player.c
	gcc -c -g player.c

omxplayer.o: omxplayer.c
	gcc -c -g omxplayer.c

clean:
	rm *.out *.o
