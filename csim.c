#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define BUF_SIZ 30

typedef struct line {
	int valid;
	int tag;
	int size;
} c_line;

typedef c_line* c_set;

typedef struct cache {
	int hit;
	int miss;
	int evict;
	c_set* sets;
} c_cache;

c_cache cache;

unsigned long long htoi(const char str[])
{
	unsigned long long result = 0;
	int i = 0;

	while(str[i] != '\0')
	{
		result *= 16;
		if (str[i] >= '0' && str[i] <= '9')
		{
			result = result + (str[i] - '0');
		}
		else if (tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f')
		{
			result = result + (tolower(str[i]) - 'a') + 10;
		}
		i++;
	}

	return result;
}

char *strrev(char *str)
{
	char *p1, *p2;

	if (! str || ! *str)
		return str;
	for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

char* get_binary(unsigned long long word)
{
	char* digits = (char *) malloc(sizeof(char)*64);
	int i = 0;

	while (word != 0) {
		digits[i++] = (word % 2) + '0';
		word /= 2;
	}
	while (i < 64) {
		digits[i++] = '0';
	}

	return strrev(digits);
}

char* hextobin(const char str[]) {
	return get_binary(htoi(str));
}

char* format_binary(char *bstring, int s, int b)
{
	printf("%s\n", bstring);
	char *formatted;
	int i;

	formatted = (char *) malloc(sizeof(char) * 66);
	assert(formatted != NULL);

	formatted[65] = '\0';

	int tag = 64 - s - b;

	for(i=0;i<tag;i++)
	{
		formatted[i] = bstring[i];
	}

	formatted[tag] = ' ';

	for(i=tag+1;i<s+tag+1;i++)
	{
		formatted[i] = bstring[i-1];
	}

	formatted[s+tag+1] = ' ';

	for(i=s+tag+2;i<b+s+tag+2;i++)
	{
		formatted[i] = bstring[i-2];
	}

	return formatted;
}

unsigned long long btoi(char *bin)
{
	int b, k, m, n;
	int len;
	unsigned long long sum;

	sum = 0;
	len = strlen(bin) - 1;

	for(k=0;k<=len;k++)
	{
		n = (bin[k] - '0'); 
		if ((n>1) || (n<0))
		{
			return 0;
		}
		for(b=1, m=len; m>k; m--)
		{
			b *= 2;
		}
		sum = sum + n * b;
	}
	return sum;
}

unsigned long long get_tag(const char* str) {
	char* temp = (char *) malloc(sizeof(char)*64);
	memset(temp, '\0', sizeof(temp));
	int i;

	for(i=0;i<66;i++) {
		if(str[i] == ' ') break;
		temp[i] = str[i];
	}
	return btoi(temp);
}

unsigned long long get_set(const char* str) {
	char* temp = (char *) malloc(sizeof(char)*64);
	int i, j = 0;

	for(i=0;i<66;i++) {
		if(str[i] == ' ') break;
	}
	for(i++;i<66;i++) {
		if (str[i] == ' ') break;
		temp[j++] = str[i];
	}

	return btoi(temp);
}

unsigned long long get_offset(const char* str) {
	char* temp = (char *) malloc(sizeof(char)*64);
	int i, j = 0;

	for(i=0;i<66;i++) {
		if(str[i] == ' ') break;
	}
	for(i++;i<66;i++) {
		if(str[i] == ' ') break;
	}
	for(i++;i<66;i++) {
		if (str[i] == ' ') break;
		temp[j++] = str[i];
	}

	return btoi(temp);
}

void run_cache(int sets, int assoc, int blocks, char* trace, int verbose) {
	char oper = 0;
	char addr[10] = {'\0'};
	char buf[BUF_SIZ] = {'\0'};
	int size = 0;
	int i, j;

	FILE *fp = fopen(trace, "r");
	if (fp == NULL) {
		printf("Error occurred while opening trace file.\n");
		exit(1);
	}

	while(fgets(buf, sizeof(buf), fp)) {
		memset(addr, '\0', sizeof(addr));
		if (buf[0] == 'I') {
			continue;
		}

		oper = buf[1];
		for(i=0;i<BUF_SIZ;i++) {
			if (buf[i+3] == ',') {
				break;
			}
		}

		for(j=0;j<i;j++) {	// FIXME : extra redundant loop. maybe due to a compiler's bug?
			addr[j] = buf[j+3];
		}

		size = atoi(&buf[i+4]);

		char* binstring = hextobin(addr);

		if (oper == 'L') {
		} else if (oper == 'S') {
		} else if (oper == 'M') {
		} else {
			printf("%c %s %d\n", oper, addr, size);
		}
	}

	fclose(fp);
	return;
}

void set_cache(int setcount, int assoc, int blocks) {
	cache.hit = 0;
	cache.miss = 0;
	cache.evict = 0;

	c_set* sets = (c_set*) malloc(sizeof(c_set) * (1 << setcount));

	int i;
	for(i=0;i<(1<<setcount);i++) {
		*(sets + i) = (c_line*) malloc(sizeof(c_line) * assoc); 
	}

	cache.sets = sets;
}

int main(int argc, char* argv[]) {
	FILE *fp;
	int sets, assoc, blocks;
	char *trace;
	int verbose = 0;

	if (argc == 10) {
		verbose = 1;
		sets = atoi(argv[3]);
		assoc = atoi(argv[5]);
		blocks = atoi(argv[7]);
		trace = argv[9];
	} else if (argc == 9) {
		verbose = 0;
		sets = atoi(argv[2]);
		assoc = atoi(argv[4]);
		blocks = atoi(argv[6]);
		trace = argv[8];
	} else {
		printf("Usage: >>./csim [-v] -s <s> -E <E> -b <b> -t <trace file>\n");
		return 1;
	}

	set_cache(sets, assoc, blocks);
	run_cache(sets, assoc, blocks, trace, verbose);

	return 0;
}