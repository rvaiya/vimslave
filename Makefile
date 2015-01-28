all:
	gcc main.c -o vimslave
install:
	install vimslave /usr/bin
