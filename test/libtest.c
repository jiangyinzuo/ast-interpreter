#include <stdio.h>
#include <stdlib.h>

int GET() {
	int a;
	scanf("%d", &a);
	return a;
}

void * MALLOC(int size) {
	return malloc(size);
}

void FREE(void * ptr) {
	return free(ptr);
}

void PRINT(int n) {
	printf("%d", n);
}