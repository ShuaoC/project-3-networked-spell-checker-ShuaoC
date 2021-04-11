# Shuao Chen, CIS 3207 Project 4
myshell: qhandle.c main.c myproject.h
	gcc -pthread -Wall qhandle.c main.c -o SpellCheckServer
