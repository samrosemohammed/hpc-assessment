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
void read_matrices(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read dimensions of Matrix A
    fscanf(file, "%d %d", &rowsA, &colsA);
    A = allocate_matrix(rowsA, colsA);
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsA; j++) {
            fscanf(file, "%lf", &A[i][j]);
        }
    }

    // Read dimensions of Matrix B
    fscanf(file, "%d %d", &rowsB, &colsB);
    B = allocate_matrix(rowsB, colsB);
    for (int i = 0; i < rowsB; i++) {
        for (int j = 0; j < colsB; j++) {
            fscanf(file, "%lf", &B[i][j]);
        }
    }

    fclose(file);
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

// Function to write the resulting matrix to a file
void write_result(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "%d %d\n", rowsC, colsC);
    for (int i = 0; i < rowsC; i++) {
        for (int j = 0; j < colsC; j++) {
            fprintf(file, "%.6lf ", C[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_threads = atoi(argv[2]);
    read_matrices(argv[1]);

    // Validate if multiplication is possible
    if (colsA != rowsB) {
        fprintf(stderr, "Error: Matrices cannot be multiplied. Dimensions do not match.\n");
        free_matrix(A, rowsA);
        free_matrix(B, rowsB);
        exit(EXIT_FAILURE);
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

    write_result("result.txt");

    // Free dynamically allocated memory
    free_matrix(A, rowsA);
    free_matrix(B, rowsB);
    free_matrix(C, rowsC);

    return 0;
}