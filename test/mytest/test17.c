extern void PRINT(int);

int main() {
	if (1) PRINT(-2);
	for (int i = 0; i < 1; i = i + 1) PRINT(-1);
	if (0) {
		PRINT(0);
	} else if (0) {
		PRINT(1);
	} else if (1) {
		PRINT(2);
	}

	if (1) 
		if (0) {
			PRINT(3);
		}
	
	if(1) if (1) PRINT(4);

	if (0) PRINT(5);
	else if (0) PRINT(6);
	else PRINT(7);
}