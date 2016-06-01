PROGNAME	:= main

include ../Makefile.common

CFLAGS		:= $(CFLAGS) \
				-Wl,-u,vfprintf -lprintf_flt

$(PROGNAME): 	iinic/iinic.c iinic/iinic.h \
				sensors_config.c sensors_config.h \
				*.h */*.h */*/*.h \
				$(PROGNAME).c
	$(CC) $(CFLAGS) iinic/iinic.c sensors_config.c $(PROGNAME).c -o $@
