int func(int a) {
	return a + 3;
}

int main() {
	int a = 0;
	a = read();
	a = func(a);
	write(a);
	return 0;
}
