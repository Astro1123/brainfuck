#include <stdio.h>			/* I/O */
#include <stdlib.h>			/* malloc(), free() */
#include <string.h>			/* strlen(), strcpy() */

#define INITIAL 1
#define CODE_SIZE 4096
#define TRUE 1
#define FALSE 0

#define QUIT 1
#define SUCCESS 0
#define ERROR -1

static long memsize;

static int makeMem(unsigned char **memory) {
	memsize = INITIAL;
	*memory = (unsigned char *)malloc(memsize * sizeof(unsigned char));
	return *memory ? TRUE : FALSE;
}

static int resizeMem(unsigned char **memory) {
	static int count = 1;
	unsigned char *tmp;

	memsize = INITIAL * (++count);
	tmp = (unsigned char *)realloc(*memory, memsize * sizeof(unsigned char));
	if (!tmp) {
		free(*memory);
	}
	return tmp ? TRUE : FALSE;
}

static int getString(char *str, long size) {
	if (fgets(str, size, stdin)) {
		if (str[strlen(str) - 1] == '\n') {
			str[strlen(str) - 1] = '\0';
		} else {
			while(getchar() != '\n');
		}
		return TRUE;
	}
	return FALSE;
}

static void initialization(unsigned char *memory) {
	long i;
	for (i = 0; i < memsize; i++) {
		memory[i] = 0;
	}
}

static int execute(unsigned char *memory, char *code) {
	unsigned long index = 0;
	unsigned long codeIndex = 0;
	char c;
	int ret = SUCCESS;
	while (TRUE) {
		c = code[codeIndex];
		if (c == 'q') {
			ret = QUIT;
			break;
		} else if (c == '\0') {
			ret = SUCCESS;
			break;
		} else if (c == '>') {
			if (++index >= memsize) {
				if (!resizeMem(&memory)) {
					printf("realloc: failed to allocate memory\n");
					ret = ERROR;
					break;
				}	
			}
		} else if (c == '<') {
			if (index-- == 0) {
				printf("pointer: out of memory\n");
				ret = ERROR;
				break;
			}
		} else if (c == '+') {
			memory[index]++;
		} else if (c == '-') {
			memory[index]--;
		} else if (c == '.') {
			printf("%c", memory[index]);
		} else if (c == ',') {
			if ((c = getchar()) == EOF) {
				printf("getchar: failed to get character\n");
				ret = ERROR;
				break;
			}
			memory[index] = (char) c;
		} else if (c == '[') {
			if (!memory[index]) {
				int count = 0;
				while (TRUE) {
					if (code[codeIndex] == '[') {
						count++;
					} else if (code[codeIndex] == ']') {
						count--;
					}
					if (!count) break;
					codeIndex++;
				}
				codeIndex--;
			}
		} else if (c == ']') {
			if (memory[index]) {
				int count = 0;
				while (TRUE) {
					if (code[codeIndex] == '[') {
						count--;
					} else if (code[codeIndex] == ']') {
						count++;
					}
					if (!count) break;
					codeIndex--;
				}
			}
		}
		if (codeIndex++ >= CODE_SIZE) break;
	}
	return ret;
}

static int inputFile(char *file, char *str, long size) {
	FILE *fp;
	char line[CODE_SIZE];

	fp = fopen(file, "r");
	if (fp == NULL) {
		return FALSE;
	}
	
	strcpy(str, "");
	while (fgets(line, sizeof(line), fp) != NULL) {
		snprintf(str, size, "%s%s", str, line);
	}
	fclose(fp);
	return TRUE;
}

int main(int argc, char *argv[]) {
	unsigned char *memory;
	char code[CODE_SIZE];
	int ret;

	if (!makeMem(&memory)) {
		printf("malloc: failed to allocate memory\n");
		return ERROR;
	}
	if (argc == 2) {
		if (!inputFile(argv[1], code, sizeof(code))) {
			printf("fopen: failed to file open\n");
			free(memory);
			return ERROR;
		}
		ret = execute(memory, code);
		if (ret == ERROR) {
			free(memory);
			return ERROR;
		}
	} else {
		do {
			printf(">> ");
			if(!getString(code, sizeof(code))) {
				printf("fgets: failed to get string\n");
				free(memory);
				return ERROR;
			}
			ret = execute(memory, code);
			initialization(memory);
			printf("\n");
			if (ret == ERROR) {
				free(memory);
				return ERROR;
			}
		} while (ret != QUIT);
	}

	free(memory);

	return SUCCESS;
}