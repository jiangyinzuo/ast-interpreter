extern void PRINT(int);
extern void* MALLOC(int);
extern void FREE(void*);

int main() {
	if (0) {
		PRINT(22);
	}
	if (9 * 2 >= 8 + 4) {
		PRINT(33);
	}
	if (1 + 5 > 6) {
		PRINT(42);
	} else {
		PRINT(99);
	}
	for (int i = 0; i < 4; i = i + 1) {
		PRINT(i);
	}
	int j = 8;
	while(1) {
		PRINT(j);
		j = j - 2;
		if (j < 0) {
			return 0;
		}
		j = j - 1;
	}
}