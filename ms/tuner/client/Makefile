PROGNAME	:= main

include ../../Makefile.common

$(PROGNAME): 	iinic/iinic.c iinic/iinic.h \
				*.h \
				$(PROGNAME).c
	$(CC) $(CFLAGS) iinic/iinic.c $(PROGNAME).c -o $@
