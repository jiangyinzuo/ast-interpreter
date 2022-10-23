extern int GET();
extern void * MALLOC(int);
extern void FREE(void *);
extern void PRINT(int);

int main() {
	int*f;
	int*g = f;
	int c[10];
	int *d = c;
	d = c + 1;
	d  = c + 2;
   int* a;
   a = (int*)MALLOC(sizeof(int)*2);
	 *(a) = 10;
	 PRINT(*a);
   *(a+1) = 20;
	 PRINT(*(a+1));
   FREE(a);
}
