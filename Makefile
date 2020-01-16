SH = shell

shell: shell.c
	gcc -Wall -ansi -o $(SH) shell.c

clean:
	rm $(SH)