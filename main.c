#include <stdio.h>
#include <errno.h>

#include "sort.h"

#define INPUT_FILE	"/home/eric/dev/data/NAVIRES-MOIS-MAREES-JOUR-IFR_2018.txt"
#define OUTPUT_FILE	"/home/eric/dev/data/NAVIRES-MOIS-MAREES-JOUR-IFR_2018.txt.sorted"
#define FIELD_DELIM	'\t'
#define KEY_FIELD	6
#define HEADER		0

static ssize_t chunk_size = (ssize_t) 1000 * (ssize_t) 1024 * (ssize_t) 1000;

int main(int argc, char **argv)
{
	return sort(INPUT_FILE, OUTPUT_FILE, chunk_size, FIELD_DELIM, KEY_FIELD,
		    HEADER);
}
