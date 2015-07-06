CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = ad_int

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)



