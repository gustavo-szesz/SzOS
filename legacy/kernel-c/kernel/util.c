#include "util.h"

void memory_copy(char *source, char *dest, int nbytes) {
    int i;
    for (i = 0; i < nbytes; i++)
    {
        *(dest + i) = *(source + i);
    }
    
} 

// K&R implementation 
void int_to_ascii(int n, char str[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n; // Record sign and make n positive
    i = 0;
    do {
        str[i++] = n % 10 + '0'; // Get next digit
    } while ((n /= 10) > 0);     // Delete it
    if (sign < 0) str[i++] = '-';
    str[i] = '\0';
    // Reverse the string
    int j;
    for (j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}