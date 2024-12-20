#include <stdio.h>
#include <string.h>
#include <unistd.h> // For the crypt function

int main() {
    char password[5]; // Buffer for the input password (4 characters + null terminator)
    char salt[12] = "$6$ab$"; // Proper salt format for SHA512

    // Prompt user for a password
    printf("Enter a 4-character password (LetterLetterNumberNumber): ");
    scanf("%4s", password);

    // Encrypt the password using SHA512 with the specified salt
    char *encrypted_password = crypt(password, salt);

    // Check if encryption succeeded
    if (encrypted_password) {
        printf("Encrypted password: %s\n", encrypted_password);
    } else {
        printf("Encryption failed.\n");
    }

    return 0;
}

