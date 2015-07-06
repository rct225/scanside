CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g
LDFLAGS = -L/usr/local/lib

MBINS = ad_dma

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -g -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)



