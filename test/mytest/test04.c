extern void PRINT(int);

int b[10];
int main() {
	int* p = b + 4;
	*p = -99;
	*p = +88;
	PRINT((*p) - 4);
}