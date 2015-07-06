CFLAGS = -I . -O2 -Wall -Wstrict-prototypes -g

MBINS = tmr_cnt_set pci-1784

all:	$(MBINS)

$(MBINS): % : %.c 
	$(CC) $(LDFLAGS) -o $@ $< -lm -ladvdaq -ldl

clean:
	-rm -f *.o $(MBINS)



