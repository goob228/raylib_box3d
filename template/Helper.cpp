#include "Helper.h"


int msa_isalpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int msa_isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

void msa_strcpy(char* dest, const char* src)
{
	while (*src) {
		*dest++ = *src++;
	}
	*dest = '\0';
}