extern void PRINT(int);

void foo(int b) {
	//{{{PRINT(b);}}}
	if (1 * 3 > 0) {
		PRINT(b - 2);
	}
}
void foo2(int x) {
	if (1) {
		foo(x + 2);
	}
}
int main() {
	foo(4);
	return 0;
}