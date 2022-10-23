extern void PRINT(int);

int b[10];
int main() {
	b[1] = 3;
	PRINT(b[1]);
	int*pb = b+1;
	*(b+2) = 3999;
	PRINT(*(b+2));
	PRINT(*pb);
	*pb = 5;
	PRINT(*pb);
	PRINT(b[1]);

	int* p = b + 4;
	*p = -99;
	*p = +88;
	PRINT((*p) - 4);
	PRINT(b[4]);
}