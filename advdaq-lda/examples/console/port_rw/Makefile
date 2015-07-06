CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = port_read port_write

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)