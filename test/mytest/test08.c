extern int GET();
extern void * MALLOC(int);
extern void FREE(void *);
extern void PRINT(int);

int main() {
   FREE(MALLOC(1));
	 PRINT(123);
}
