CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = cnt_compare

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -g -o $@ $< -lm -ladvdaq -ldl -lpthread

clean:
	-rm -f *.o $(MBINS)



