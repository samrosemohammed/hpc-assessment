#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 4 // Number of threads hardcoded to 4

int count = 0;
char salt[7];
char encrypted[100];
int password_found = 0; // Shared flag to indicate whether the password is found
char password[7];       // To store the actual password

pthread_mutex_t mutex; // Pthread mutex for synchronization
clock_t start_time, end_time;

FILE *output_file; // File pointer for logging results

typedef struct
{
    int start;
    int end;
} divide;

void substr(char *dest, char *src, int start, int length)
{
    memcpy(dest, src + start, length);
    dest[length] = '\0';
}

void *crack(void *arg)
{
    divide *slice = (divide *)arg;
    int i, j, k;
    char plain[7];
    char salt_and_plain[20]; // Make sure to adjust the size as needed
    char *enc;

    struct crypt_data data;
    data.initialized = 0;

    // Print which thread is working on which portion of the search space
    printf("Thread %ld working on range: %c%c to %c%c\n", pthread_self(), 'A' + slice->start, 'A', 'A' + slice->end, 'Z');

    for (i = 'A' + slice->start; i <= 'A' + slice->end; i++)
    {
        for (j = 'A'; j <= 'Z'; j++)
        {
            for (k = 0; k <= 99; k++)
            {
                // Generate the candidate password
                sprintf(plain, "%c%c%02d", i, j, k);
                snprintf(salt_and_plain, sizeof(salt_and_plain), "%s%s", salt, plain);

                // Print the password being tested by this thread
                printf("Thread %ld testing: %s\n", pthread_self(), plain);

                enc = crypt_r(plain, salt, &data);

                // Use the mutex to protect the critical section
                pthread_mutex_lock(&mutex);
                {
                    count++;
                    if (strcmp(encrypted, enc) == 0) // Check if the decrypted password matches the encrypted password
                    {
                        printf("Thread %ld found the password: %s\n", pthread_self(), plain); // if the password matches
                        fprintf(output_file, "Thread %ld found the password: %s\n", pthread_self(), plain); // Write to file
                        strncpy(password, plain, sizeof(password) - 1);
                        password_found = 1; // Change value to 1 when password is found
                    }
                }
                pthread_mutex_unlock(&mutex);

                if (password_found)
                {
                    // Exit the thread when the password is found
                    pthread_exit(NULL);
                }
            }
        }
    }

    printf("Thread %ld finished its task.\n", pthread_self()); // Indicate the thread is done
    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    if (argc != 2) // checks for user input
    {
        fprintf(stderr, "Usage: %s <encrypted>\n", argv[0]); // runs when the input isn't as expected
        return 1;
    }

    strncpy(encrypted, argv[1], sizeof(encrypted) - 1); // copy the encrypted code
    substr(salt, encrypted, 0, 6);

    output_file = fopen("results.txt", "w"); // Open the file for writing
    if (output_file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    pthread_t threads[NUM_THREADS]; // define number of threads
    start_time = clock();           // records time during the start of the cracking password
    pthread_mutex_init(&mutex, NULL);
    divide slices[NUM_THREADS];
    int slice_size = 26 / NUM_THREADS; // Assuming letters 'A' to 'Z'

    for (int i = 0; i < NUM_THREADS; i++) // slicing the thread
    {
        slices[i].start = i * slice_size;
        slices[i].end = (i + 1) * slice_size - 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) // creates number of thread assigned by the user
    {
        if (pthread_create(&threads[i], NULL, crack, (void *)&slices[i]) != 0)
        {
            fprintf(stderr, "Error creating thread %d\n", i); // if creation of thread is unsuccessful
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) // wait for all thread to finish
    {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    end_time = clock(); // records the time at the end of all execution

    if (password_found) // displays if the password is found
    {
        printf("Encrypted password= %s\n", encrypted); // display encrypted password
        printf("Password:%s\n", password);             // displays cracked password

        fprintf(output_file, "Encrypted password= %s\n", encrypted); // Write to file
        fprintf(output_file, "Password:%s\n", password);             // Write to file
    }
    else
    {
        printf("add '\\' before $ in encrypted password\n"); // if the password is not found
        fprintf(output_file, "Password not found. Ensure correct format with escaped $ symbols.\n");
    }

    printf("%d solutions explored\n", count);                                 // display total number of solutions
    fprintf(output_file, "%d solutions explored\n", count);                 // Write to file

    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC; // calculate elapsed time
    printf("Elapsed time: %.4f seconds\n", elapsed_time);                     // displays the time
    fprintf(output_file, "Elapsed time: %.4f seconds\n", elapsed_time);       // Write to file

    fclose(output_file); // Close the file
    return 0;
}