/*
------------------DR VASILIOS KELEFOURAS-----------------------------------------------------
------------------COMP1001 ------------------------------------------------------------------
------------------COMPUTER SYSTEMS MODULE-------------------------------------------------
------------------UNIVERSITY OF PLYMOUTH, SCHOOL OF ENGINEERING, COMPUTING AND MATHEMATICS---
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>

// Function declarations
void Gaussian_Blur();
void Sobel();
void initialize_kernel();
void read_image(const char* filename);
void write_image(const char* filename, unsigned char* output_image);
void openfile(const char* filename, FILE** finput);
int getint(FILE* fp);
void allocate_memory();

// Dynamic variables for image dimensions
int M; // Number of columns
int N; // Number of rows

// Dynamic arrays
unsigned char* frame1 = NULL;
unsigned char* filt = NULL;
unsigned char* gradient = NULL;

// Masks
const signed char Mask[5][5] = {
    {2, 4, 5, 4, 2},
    {4, 9, 12, 9, 4},
    {5, 12, 15, 12, 5},
    {4, 9, 12, 9, 4},
    {2, 4, 5, 4, 2}
};

const signed char GxMask[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

const signed char GyMask[3][3] = {
    {-1, -2, -1},
    {0,  0,  0},
    {1,  2,  1}
};

char header[100];
errno_t err;

int main() {
    char input_path[256];
    char output_path_blur[256];
    char output_path_edge[256];

    for (int i = 1; i <= 31; i++) {
        // Create dynamic paths for input and output files
        sprintf_s(input_path, "C:/Users/Rhys/Documents/q3/q3a/image_processing/input_images/a%d.pgm", i);
        //image_processing/input_images/a%d.pgm

        sprintf_s(output_path_blur, "C:/Users/Rhys/Documents/q3/q3a/image_processing/output_images/blurred%d.pgm", i);
        //sprintf_s(output_path_blur, "output_images/blurred%d.pgm", i);

        sprintf_s(output_path_edge, "C:/Users/Rhys/Documents/q3/q3a/image_processing/output_images/edge_detection%d.pgm", i);
        //sprintf_s(output_path_edge, "output_images/edge_detection%d.pgm", i);

        // Open the input file and read image dimensions
        FILE* finput = NULL;
        openfile(input_path, &finput);
        fclose(finput);

        // Allocate memory dynamically for the current image dimensions
        allocate_memory();

        // Read image data into frame1
        read_image(input_path);

        // Perform Gaussian blur
        Gaussian_Blur();

        // Perform edge detection
        Sobel();

        // Write output images
        write_image(output_path_blur, filt);
        write_image(output_path_edge, gradient);

        printf("Processed image %d successfully.\n", i);
    }

    // Free dynamically allocated memory
    free(frame1);
    free(filt);
    free(gradient);

    return 0;
}

void Gaussian_Blur() {
    int row, col, rowOffset, colOffset;
    int newPixel;
    unsigned char pix;
    const unsigned short int size = 2;

    for (row = 0; row < N; row++) {
        for (col = 0; col < M; col++) {
            newPixel = 0;
            for (rowOffset = -size; rowOffset <= size; rowOffset++) {
                for (colOffset = -size; colOffset <= size; colOffset++) {
                    if ((row + rowOffset < 0) || (row + rowOffset >= N) ||
                        (col + colOffset < 0) || (col + colOffset >= M)) {
                        pix = 0;
                    }
                    else {
                        pix = frame1[M * (row + rowOffset) + (col + colOffset)];
                    }
                    newPixel += pix * Mask[size + rowOffset][size + colOffset];
                }
            }
            filt[M * row + col] = (unsigned char)(newPixel / 159);
        }
    }
}

void Sobel() {
    int row, col, rowOffset, colOffset;
    int Gx, Gy;

    for (row = 1; row < N - 1; row++) {
        for (col = 1; col < M - 1; col++) {
            Gx = 0;
            Gy = 0;

            for (rowOffset = -1; rowOffset <= 1; rowOffset++) {
                for (colOffset = -1; colOffset <= 1; colOffset++) {
                    Gx += filt[M * (row + rowOffset) + col + colOffset] * GxMask[rowOffset + 1][colOffset + 1];
                    Gy += filt[M * (row + rowOffset) + col + colOffset] * GyMask[rowOffset + 1][colOffset + 1];
                }
            }
            gradient[M * row + col] = (unsigned char)sqrt(Gx * Gx + Gy * Gy);
        }
    }
}

void read_image(const char* filename) {
    FILE* finput;
    int i, j, temp;

    printf("\nReading %s image from disk ...", filename);
    finput = NULL;
    openfile(filename, &finput);

    if ((header[0] == 'P') && (header[1] == '5')) {
        for (j = 0; j < N; j++) {
            for (i = 0; i < M; i++) {
                temp = getc(finput);
                if (temp == EOF) {
                    fprintf(stderr, "Error reading pixel data\n");
                    exit(EXIT_FAILURE);
                }
                frame1[M * j + i] = (unsigned char)temp;
            }
        }
    }
    else if ((header[0] == 'P') && (header[1] == '2')) {
        for (j = 0; j < N; j++) {
            for (i = 0; i < M; i++) {
                if (fscanf_s(finput, "%d", &temp) == EOF) {
                    fprintf(stderr, "Error reading pixel data\n");
                    exit(EXIT_FAILURE);
                }
                frame1[M * j + i] = (unsigned char)temp;
            }
        }
    }
    else {
        fprintf(stderr, "Problem with reading the image.\n");
        exit(EXIT_FAILURE);
    }

    fclose(finput);
    printf("\nImage successfully read from disk.\n");
}

void write_image(const char* filename, unsigned char* output_image) {
    FILE* foutput;
    int i, j;

    printf("Writing result to disk ...\n");

    if ((err = fopen_s(&foutput, filename, "wb")) != NULL) {
        fprintf(stderr, "Unable to open file %s for writing\n", filename);
        exit(-1);
    }

    fprintf(foutput, "P2\n");
    fprintf(foutput, "%d %d\n", M, N);
    fprintf(foutput, "255\n");

    for (j = 0; j < N; ++j) {
        for (i = 0; i < M; ++i) {
            fprintf(foutput, "%3d ", output_image[M * j + i]);
            if (i % 32 == 31) fprintf(foutput, "\n");
        }
        if (M % 32 != 0) fprintf(foutput, "\n");
    }
    fclose(foutput);
}

void openfile(const char* filename, FILE** finput) {
    if ((err = fopen_s(finput, filename, "rb")) != NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", filename);
        exit(-1);
    }

    if (fscanf_s(*finput, "%s", header, (unsigned)_countof(header)) == EOF) {
        fprintf(stderr, "Error reading file header\n");
        exit(EXIT_FAILURE);
    }
    M = getint(*finput);
    N = getint(*finput);

    printf("\tHeader is %s, while M=%d, N=%d\n", header, M, N);

    getint(*finput); // Skip max pixel value
}

void allocate_memory() {
    size_t image_size = M * N;

    unsigned char* temp = (unsigned char*)realloc(frame1, image_size * sizeof(unsigned char));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for frame1!\n");
        exit(EXIT_FAILURE);
    }
    frame1 = temp;

    temp = (unsigned char*)realloc(filt, image_size * sizeof(unsigned char));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for filt!\n");
        exit(EXIT_FAILURE);
    }
    filt = temp;

    temp = (unsigned char*)realloc(gradient, image_size * sizeof(unsigned char));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for gradient!\n");
        exit(EXIT_FAILURE);
    }
    gradient = temp;
}

int getint(FILE* fp) {
    int c, i = 0;

    do { c = getc(fp); } while (c < '0' || c > '9');
    while (c >= '0' && c <= '9') {
        i = (i * 10) + (c - '0');
        c = getc(fp);
    }
    return i;
}
