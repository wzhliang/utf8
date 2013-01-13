CC=gcc
COMPILE=${CC} -c -Wall -g
LINK=${CC} -o $@ $+

%.o:%.c
	${COMPILE} $<

all:utf8stat ideo

ideo:ideo.o
	${LINK}

utf8stat:utf8stat.o
	${LINK}


clean:
	rm ideo.o utf8stat.o ideo utf8stat

