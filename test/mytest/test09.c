extern void PRINT(int);
extern void* MALLOC(int);
extern void FREE(void*);
int main() {
	int a[10];
	*(a + 1) = 1;
	PRINT(a[1]);
	PRINT(*(a+1));
	int* b =(int*) MALLOC(100);
	*(b+1) = 2;
	PRINT( *(b+1));
	FREE(b+4-4);
}