int func(int a[5]) {
	return 1;
}

int func(int a[10]);
int main(){
	int b[15];
	func(b);
	return 0;
}
