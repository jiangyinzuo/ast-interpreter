extern void PRINT(int);

void hello() {PRINT(42);}
void world() {
	PRINT(99);
	hello();
}

void myPrint(int a) {
	PRINT(a);
}

int add2(int b) {
	return b + 2;
}

int a = 1;
void printGlobal() {
	PRINT(a);
}
int main() {
	PRINT(a);
	int a = 2;
	PRINT(a);
	{
		PRINT(a);
		int a = 3;
		PRINT(a);
	}
	PRINT(a);
	a = 4;
	PRINT(a);
	hello();
	world();
	myPrint(a + 1);
	myPrint(add2(a));
	printGlobal();
}