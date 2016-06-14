PROGNAME	:= main

include ../Makefile.common

CFLAGS		:= $(CFLAGS) -I$(PWD)/../iinic_wrapper

$(PROGNAME): 	../iinic_wrapper/iinic/iinic.c ../iinic_wrapper/iinic/iinic.h \
				../iinic_wrapper/iinic_wrapper.h ../iinic_wrapper/usart_complex.h \
				$(PROGNAME).c
	$(CC) $(CFLAGS) ../iinic_wrapper/iinic/iinic.c $(PROGNAME).c -o $@
