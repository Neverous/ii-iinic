PROGNAME	:= test

include ../Makefile.common

$(PROGNAME): 	iinic/iinic.c iinic/iinic.h \
				iinic_wrapper.h \
				$(PROGNAME).c
	$(CC) $(CFLAGS) iinic/iinic.c $(PROGNAME).c -o $@
