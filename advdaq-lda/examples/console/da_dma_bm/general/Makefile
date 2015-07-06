CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = da_dma_bm

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)