extern void PRINT(int);

int b[10];
int main() {
	int a[10];
	a[2] = 4;
	a[1] = a[2] * 2;
	PRINT(a[2]);
	PRINT(a[1]);
	b[1] = 99;
	PRINT(b[1]);
}