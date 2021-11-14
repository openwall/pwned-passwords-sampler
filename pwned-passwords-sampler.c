/*
 * Copyright (c) 2021 by Solar Designer
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAXI 3650716681ULL /* exact figure for HIBP v7 */
#define SAMPLE 1000000

int main(void) {
	struct stat st;
	if (fstat(0, &st)) {
		perror("fstat");
		return 1;
	}

	char *p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, 0, 0);
	if (p == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	if (madvise(p, st.st_size, MADV_SEQUENTIAL | MADV_WILLNEED)) {
		perror("madvise");
		return 1;
	}

	const uint64_t as = MAXI * sizeof(uint64_t);
	assert(as == (size_t)as); /* fail on 32-bit */
	uint64_t * const i2o = malloc(as);

	const char *pp = p;
	uint64_t *ip = i2o;
	unsigned long long total = 0;
	while (pp + 36 <= p + st.st_size) {
		if (!((ip - i2o) & 0xffffff))
			fprintf(stderr, "\r%.0f%%", 100. * (pp - p) / st.st_size);
		if (pp[32] != ':') {
bad_file:
			fprintf(stderr, "\rInput file format error\n");
			return 1;
		}
		char *e;
		unsigned long c = strtoul(pp + 33, &e, 10);
		if (e[0] != '\r' || e[1] != '\n' || c < 1)
			goto bad_file;
		total += c;
		if (total > MAXI) {
			fprintf(stderr, "\rTotal exceeds allocation\n");
			return 1;
		}
		while (c--)
			*ip++ = pp - p;
		pp = e + 2;
	}
	if (pp != p + st.st_size)
		goto bad_file;
	fprintf(stderr, "\rTotal %llu\n", total);

	if (madvise(p, st.st_size, MADV_RANDOM | MADV_WILLNEED)) {
		perror("madvise");
		return 1;
	}

	uint64_t mask;
	{
		uint64_t x = total, y;
		while ((y = x & (x - 1)))
			x = y;
		if (x == total)
			mask = x - 1;
		else
			mask = (x << 1) - 1;
	}

	int rfd = open("/dev/urandom", O_RDONLY);
	unsigned int n = SAMPLE;
	do {
		uint64_t rnd;
		if (read(rfd, &rnd, sizeof(rnd)) != sizeof(rnd))
			continue;
		rnd &= mask;
		if (rnd >= total || i2o[rnd] == 1)
			continue;
		pp = p + i2o[rnd];
		i2o[rnd] = 1; /* taken */
		while (*pp != '\r')
			putchar(*pp++);
		putchar('\n');
		n--;
	} while (n);
	close(rfd);

	return 0;
}
