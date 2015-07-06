CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = do_soft_wd

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -o $@ $< -lm -ladvdaq -ldl -lpthread

clean:
	-rm -f *.o $(MBINS)



