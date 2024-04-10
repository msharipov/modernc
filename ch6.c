/* TODO :
*   - vector-vector multiplication
*   - matrix-vector multiplication
*   - matrix inverse
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void
print_vec(const size_t N, const double vec[static N]) {

    if (!N) {

        return;
    }

    for (size_t i = 0; i < N - 1; i++) {

        printf("%f, ", vec[i]);
    }

    printf("%f", vec[N - 1]);
}


double
dot(const size_t N, const double vec1[static N],
    const double vec2[static N]) {
    
    double result = 0;

    for (size_t i = 0; i < N; i++) {
        
        result += vec1[i]*vec2[i];
    }

    return result;
}


// Multiplies an N by M matrix by a vector of length M.
void
matvec_mult(const size_t N, const size_t M, const double mat[static N][M],
            const double vec2[static M], double result[static N]) {

    for (size_t n = 0; n < N; n++) {
        
        result[n] = dot(M, mat[n], vec2);
    }
}


void
mult_vec(const size_t N, double vec[static N], const double mult) {
    
    for (size_t i = 0; i < N; i++) {

        vec[i] *= mult;
    }
}


void
addmult_vec(const size_t N, double dest[static N],
            const double from[static N], const double mult) {

    if (dest == from) {

        mult_vec(N, dest, mult + 1);
        return;
    }

    for (size_t i = 0; i < N; i++) {

        dest[i] += mult*from[i];
    }
}


// Computes the inverse of an N by N matrix, using Gaussian elimination
// Returns 0 if OK, 1 if operation fails
int
mat_inv(const size_t N, const double init_mat[static N*N],
        double inv[static N][N]) {
    
    const size_t mat_size = sizeof(double[N*N]);
    double* const mat = malloc(mat_size);

    if (mat) {

        memcpy(mat, init_mat, mat_size);

    } else {

        return 1;
    }

    // Initialize inv to be an identity matrix:
    for (size_t r = 0; r < N; r++) {

        for (size_t c = 0; c < N; c++) {
            
            inv[r][c] = (r == c);
        }
    }
    
    for (size_t c = 0; c < N; c++) {
        
        // Normalize the initial row
        const double row_lead_inv = 1.0/mat[c*N + c];

        if (fpclassify(row_lead_inv) == FP_INFINITE) {

            free(mat);
            return 1;
        }
    
        mult_vec(N, mat + c*N, row_lead_inv);
        mult_vec(N, inv[c], row_lead_inv);

        for (size_t r = 0; r < N; r++) {
            
            if (r == c) {
                continue;
            }
            
            // Cancel out element c in other rows
            const double mult_inv = -mat[r*N + c];

            addmult_vec(N, mat + r*N, mat + c*N, mult_inv);
            addmult_vec(N, inv[r], inv[c], mult_inv);
        }
    }

    free(mat);
    return 0;
}


int
main() {

    #define LEN 5
    #define MAT_R 4
    #define MAT_C LEN

    const double A[LEN] = {1, 3, -3, 2, -5};
    const double B[LEN] = {2, -4, -2, 1, -2};
    const double M[MAT_R][MAT_C] = {
        { 1,  4, -4,  5,  2},
        { 2, -3,  0,  1, -1},
        { 4,  2,  2, -5,  1},
        {-1,  4, -1,  2,  3}
    };
    const double IM[LEN][LEN] = {
        {1, 0, 0, 0, 0},
        {0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1}
    };
    const double M2[3][3] = {
        {6, -5,  2},
        {3, -4, -2},
        {-1, 3,  5}
    };

    double MA[MAT_R] = {0};
    double MB[MAT_R] = {0};
    double IM_inv[LEN][LEN] = {0};
    double M2_inv[3][3] = {0};
    double M2_inv_inv[3][3] = {0};
    
    matvec_mult(MAT_R, MAT_C, M, A, MA);
    matvec_mult(MAT_R, MAT_C, M, B, MB);
    mat_inv(LEN, (const double *)IM, IM_inv);
    mat_inv(3, (const double *)M2, M2_inv);
    mat_inv(3, (const double *)M2_inv, M2_inv_inv);

    printf("A = [");
    print_vec(LEN, A);
    printf("]\n");

    printf("B = [");
    print_vec(LEN, B);
    printf("]\n");

    printf("M = [\n");
    for (size_t r = 0; r < MAT_R; r++) {
        print_vec(MAT_C, M[r]);
        printf("\n");
    }
    printf("]\n\n");

    printf("A * B = %f\n", dot(LEN, A, B));
    
    printf("MA = [");
    print_vec(MAT_R, MA);
    printf("]\n");
    
    printf("MB = [");
    print_vec(MAT_R, MB);
    printf("]\n\n");

    printf("I = [\n");
    for (size_t r = 0; r < LEN; r++) {
        print_vec(LEN, IM[r]);
        printf("\n");
    }
    printf("]\n\n");
    
    printf("inv(I) = [\n");
    for (size_t r = 0; r < LEN; r++) {
        print_vec(LEN, IM_inv[r]);
        printf("\n");
    }
    printf("]\n\n");

    printf("M2 = [\n");
    for (size_t r = 0; r < 3; r++) {
        print_vec(3, M2[r]);
        printf("\n");
    }
    printf("]\n\n");

    printf("inv(M2) = [\n");
    for (size_t r = 0; r < 3; r++) {
        print_vec(3, M2_inv[r]);
        printf("\n");
    }
    printf("]\n\n");

    printf("inv(inv(M2)) = [\n");
    for (size_t r = 0; r < 3; r++) {
        print_vec(3, M2_inv_inv[r]);
        printf("\n");
    }
    printf("]\n\n");

    exit(EXIT_SUCCESS);
}
