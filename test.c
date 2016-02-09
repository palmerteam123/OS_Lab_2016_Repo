#include <stdio.h>
#include <string.h>

void main()
{
	char m[1024];

	sprintf(m,"%d  ",3);

	int N;
sscanf(m,"%d",&N);
	printf("%d!\n",N);
}