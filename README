-------------------------------------------------------
Axel Wahlstrom's README.txt File for CS457 Chat Program
-------------------------------------------------------
Written in the C programming language by Axel Wahlstrom
using Beej's Guide to Network Programming
-------------------------------------------------------
Overview: This is a chat program for CS457 to establish
TCP client/server connections to allow the client and
server to send message packets back and forth.

The packets are assembled in the following format
before being sent:

[Packet Version 16-bits : 457][string-length 16-bits]
[actual message <= 140-bytes ASCII]
-------------------------------------------------------
Building:
This program comes with 3 files, README, chat.c, and
a Makefile.

To build this program simply type "make"
in the command line, without the quatations,
in the directory where the 3 files stated above are
located.
-------------------------------------------------------
Running:
After running the makefile, an executable called
"chat" will appear in the directory.

You may run the program in the following ways:

./chat - this will simply run the server, and display
the port and ip the server is listening on - give the
ip and port to your friend!

./chat -p <port> -s <ip-number> - this will run the
client. Use parameters -p to specify port number
and -s to specify ip number of the server - enter
the ip and port your friend gives you!
*NOTE: the parameters can be taken in any order.

./chat -h - this will run the client with usage help.
If you ever get confused as to how to use the params,
just type this!
*NOTE: this will not run a client it will only give
you program usage!

./chat -p <port> -s <ip-number> -h - this will run
the client with usage help. If you ever get confused
as to how to use the parameters, just type this!
*NOTE: although port and ip are specified it will
still produce usage help 

