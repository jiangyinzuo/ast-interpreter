extern void PRINT(int);

int add4(int c) {
	return c + 4;
}
int main() {
	int a = 1;
	int b = add4(a);
	PRINT(99999);
	PRINT(b);
	PRINT(99999);
}