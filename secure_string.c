#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Node
{
	struct Node *previous;
	char *value;
	struct Node *next;
} NODE;

typedef struct SecureString
{
	char *data[0];
} SECURE_STRING;

void append(SECURE_STRING *string, char value)
{
}

int _main(int argc, char *argv[])
{
	printf("Hello");
	return EXIT_SUCCESS;
}