all: clean i3-volume-control

i3-volume-control.o:
	gcc -c -Wall -g main.c -o i3-volume-control.o

i3-volume-control: i3-volume-control.o
	gcc i3-volume-control.o `pkg-config --cflags --libs alsa` -g -o i3-volume-control
clean:
	rm -fv i3-volume-control.o i3-volume-control
