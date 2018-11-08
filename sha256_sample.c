#include <stdio.h>
#include <openssl/sha.h>
#include <string.h>

int main()
{
	char str[] = "test.txt";
	unsigned char h2[SHA256_DIGEST_LENGTH];
	unsigned char hash[SHA256_DIGEST_LENGTH]; 
	int i;

	SHA256(str, strlen(str), hash);

	for (i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		printf("%02x", hash[i]);
		h2[i] = hash[i];
	}
	printf("\n");
	for (i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		printf("%02x", h2[i]);
	}
	printf("\nSHA256 Length: %d\n", SHA256_DIGEST_LENGTH);

	return 0;
}
