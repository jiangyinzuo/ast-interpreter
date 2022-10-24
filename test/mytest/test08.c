extern int GET();
extern void * MALLOC(int);
extern void FREE(void *);
extern void PRINT(int);

int main() {
   FREE(MALLOC(1));
	 PRINT(123);
	 int** a[10];
	 a[1] = (int**)MALLOC(16);
	 a[1][1] = (int*)MALLOC(16);
	 a[1][1][1] = 3;
	 int***p=a;
	 PRINT(p[1][1][1]);
	 int*b = p[1][1];
	 p[1][1][0] = 4;
		PRINT(*(b));
		FREE(a[1]);FREE(b);
}
