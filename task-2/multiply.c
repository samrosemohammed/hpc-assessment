#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Global variables for matrices and dimensions
double **A, **B, **C;
int rowsA, colsA, rowsB, colsB, rowsC, colsC, num_threads;

// Function to allocate a matrix dynamically
double **allocate_matrix(int rows, int cols) {
    double **matrix = (double **)malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (double *)malloc(cols * sizeof(double));
    }
    return matrix;
}

// Function to free allocated memory for a matrix
void free_matrix(double **matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Function to read matrices from a file
int read_matrices(FILE *file) {
    // Read dimensions of Matrix A
    if (fscanf(file, "%d %d", &rowsA, &colsA) != 2) return 0;
    A = allocate_matrix(rowsA, colsA);
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsA; j++) {
            if (fscanf(file, "%lf", &A[i][j]) != 1) return 0;
        }
    }

    // Read dimensions of Matrix B
    if (fscanf(file, "%d %d", &rowsB, &colsB) != 2) return 0;
    B = allocate_matrix(rowsB, colsB);
    for (int i = 0; i < rowsB; i++) {
        for (int j = 0; j < colsB; j++) {
            if (fscanf(file, "%lf", &B[i][j]) != 1) return 0;
        }
    }

    return 1;
}

// Function to compute part of the result matrix in a thread
void *multiply(void *arg) {
    int thread_id = *(int *)arg;
    for (int i = thread_id; i < rowsC; i += num_threads) {
        for (int j = 0; j < colsC; j++) {
            C[i][j] = 0;
            for (int k = 0; k < colsA; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return NULL;
}

// Function to write the resulting matrix or error to a file
void write_result(FILE *file, const char *error_message) {
    if (error_message) {
        fprintf(file, "%s\n", error_message);
    } else {
        fprintf(file, "%d %d\n", rowsC, colsC);
        for (int i = 0; i < rowsC; i++) {
            for (int j = 0; j < colsC; j++) {
                fprintf(file, "%.6lf ", C[i][j]);
            }
            fprintf(file, "\n");
        }
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_threads = atoi(argv[2]);
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    FILE *result_file = fopen("result.txt", "w");
    if (!result_file) {
        perror("Error opening result file");
        exit(EXIT_FAILURE);
    }

    while (read_matrices(file)) {
        // Validate if multiplication is possible
        if (colsA != rowsB) {
            write_result(result_file, "Error: Matrices cannot be multiplied. Dimensions do not match.");
            free_matrix(A, rowsA);
            free_matrix(B, rowsB);
            continue;  // Skip to next matrix pair
        }

        // Set dimensions for the result matrix
        rowsC = rowsA;
        colsC = colsB;
        C = allocate_matrix(rowsC, colsC);

        // Adjust thread count if necessary
        if (num_threads > rowsC) {
            num_threads = rowsC;
        }

        pthread_t threads[num_threads];
        int thread_ids[num_threads];
        for (int i = 0; i < num_threads; i++) {
            thread_ids[i] = i;
            pthread_create(&threads[i], NULL, multiply, &thread_ids[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }

        write_result(result_file, NULL);  // Write the result matrix

        // Free dynamically allocated memory
        free_matrix(A, rowsA);
        free_matrix(B, rowsB);
        free_matrix(C, rowsC);
    }

    fclose(file);
    fclose(result_file);

    return 0;
}
