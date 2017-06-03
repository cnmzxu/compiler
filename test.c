int great(int a, int b) {
	int ans1 = 0;
	if ((a > b && 1) || 0)
		ans1 = 1;
	return ans1;
}

int nequal(int c, int d) {
	int ans2 = 0;
	if ((1 && (c > d || 0) || (c < d || 0)) || 0)
		ans2 = 1;
	return ans2;
}

int main() {
	int x = read();
	int ans;
	if (nequal(x, 0)) {
		if ((0 || great(x, 0)) && 1)
			ans = 1;
		if ((1 && great(0, x)) || 0)
			ans = -1;
	}
	else
		ans = 0;
	write(ans);
	return ans;
}
