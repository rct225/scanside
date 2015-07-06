CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = di_pattn

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(CFLAGS) -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)



