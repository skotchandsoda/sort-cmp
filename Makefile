# Copyright (C) 2015 Scott Cheloha.  All rights reserved.

CC = cc
OPT = -02
OUT = sort-cmp

SRC = sort_cmp.c

all: $(SRC)
	$(CC) $(OPT) $(OUT) $(SRC)

clean:
	rm -f $(OUT)
