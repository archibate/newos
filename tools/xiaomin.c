#include <stdio.h>
#include <string.h>

int dp[2][6666];

int m(int a, int b)
{
	return b && (a == -1 || b < a) ? b : a;
}

int main(void)
{
	int k, i;
	printf("...\n");
	memset(dp, -1, sizeof(dp));
	dp[0][2333] = 0;

	for (k = 0; dp[k][2333+1] == -1; k = !k)
		for (i = 233; i < 6000; i++)
			dp[k][i] = m(m(dp[k][i], dp[!k][i-97] + 1), dp[!k][i+127] + 1);
	printf("%d\n", dp[k][2333+1]);
	return 0;
}
