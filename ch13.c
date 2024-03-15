/*
*   TODO:
*   - complex root finder using Newton's method = DONE
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdbool.h>

typedef double complex doubleC;

enum Error_Type {
    
    SUCCESS = 0,
    UNDEFINED = 1,
    UNSTABLE = 2
};

typedef struct Cmpl_Deriv_Result Cmpl_Deriv_Result;
struct Cmpl_Deriv_Result {

    doubleC value;
    enum Error_Type error;
};

typedef struct Newton_Root_Result Newton_Root_Result;
struct Newton_Root_Result {

    doubleC value;
    enum Error_Type error;
};


// Given z = x + iy and f(z) = u(x,y) + iv(x,y), computes f'(z) as:
// du/dx + i*dv/dx with both derivatives evaluated at z_0
Cmpl_Deriv_Result 
cmpl_deriv(doubleC(*f)(const doubleC), const doubleC z, const double dz) {
    
    const double h = fabs(dz/2);

    const doubleC f1 = f(z - 2*(doubleC)h);
    const doubleC f2 = f(z - (doubleC)h);
    const doubleC f3 = f(z);
    const doubleC f4 = f(z + (doubleC)h);
    const doubleC f5 = f(z + 2*(doubleC)h);

    const double u1 = creal(f1);
    const double u2 = creal(f2);
    const double u3 = creal(f3);
    const double u4 = creal(f4);
    const double u5 = creal(f5);

    const double v1 = cimag(f1);
    const double v2 = cimag(f2);
    const double v3 = cimag(f3);
    const double v4 = cimag(f4);
    const double v5 = cimag(f5);

    Cmpl_Deriv_Result result = {0, SUCCESS};
    
    const double dudx = (u1 - 8*u2 + 8*u4 - u5) / (12*h);
    const double dvdx = (v1 - 8*v2 + 8*v4 - v5) / (12*h);

    if (isnan(dudx) || isinf(dudx) || isnan(dvdx) || isinf(dvdx) ||
        isnan(u3) || isinf(u3) || isnan(v3) || isinf(v3)) {

        result.error = UNDEFINED;
        return result;
    }
    
    result.value = (doubleC)dudx + I*(doubleC)dvdx;

    const bool u_monotonic = (u1 < u2 && u2 < u3 && u3 < u4 && u4 < u5) ||
                             (u1 > u2 && u2 > u3 && u3 > u4 && u4 > u5);
    const bool v_monotonic = (v1 < v2 && v2 < v3 && v3 < v4 && v4 < v5) ||
                             (v1 > v2 && v2 > v3 && v3 > v4 && v4 > v5);

    if (u_monotonic && v_monotonic) { 
        return result;
    }
    
    const bool u_right_hump = (u4 > u3 && u4 > u5) || (u4 < u3 && u4 < u5);
    const bool u_left_hump = (u2 > u1 && u2 > u3) || (u2 < u1 && u2 < u3); 

    if (u_right_hump) {
        
        if (u_left_hump || fabs(2*u4 - u3 - u5) > fabs(u3 - u1)) { 
            
            result.error = UNSTABLE;
            return result;
        }
    }

    if (u_left_hump) {
        
        if (u_right_hump || fabs(2*u2 - u1 - u3) > fabs(u5 - u3)) {
            
            result.error = UNSTABLE;
            return result;
        }
        
    }

    const bool v_right_hump = (v4 > v3 && v4 > v5) || (v4 < v3 && v4 < v5);
    const bool v_left_hump = (v2 > v1 && v2 > v3) || (v2 < v1 && v2 < v3); 

    if (v_right_hump) {
        
        if (v_left_hump || fabs(2*v4 - v3 - v5) > fabs(v3 - v1)) { 
            
            result.error = UNSTABLE;
            return result;
        }
    }

    if (v_left_hump) {
        
        if (v_right_hump || fabs(2*v2 - v1 - v3) > fabs(v5 - v3)) {
            
            result.error = UNSTABLE;
            return result;
        }
        
    }
    
    return result;
}


doubleC
poly1(const doubleC z) {
    
    return z;
}


doubleC
poly2(const doubleC z) {
    
    return z*z*z - 2*z*z + 5*z + 4;
}


Newton_Root_Result
find_root(doubleC(*f)(const doubleC), doubleC guess, double eps) {
    
    Newton_Root_Result result = {0};
    Cmpl_Deriv_Result deriv = {0};
    doubleC change = 0;
    doubleC f_guess = 0;
    
    do {

        deriv = cmpl_deriv(f, guess, eps);
        if (deriv.error) {

            result.error = deriv.error;
            return result;
        }

        //doubleC magnitude = cabs(deriv.value);
        f_guess = f(guess);
        if (isnan(creal(f_guess)) || isnan(cimag(f_guess)) ||
            isinf(creal(f_guess)) || isinf(cimag(f_guess))) {

            result.error = UNDEFINED;
            return result;
        }

        change = f_guess / deriv.value;
        

        guess -= change;

    } while (cabs(f_guess) > eps);
    
    result.value = guess;
    return result;
}

int
main() {
    
    Newton_Root_Result res1 = find_root(poly1, 1 + I, 1e-6);
    if (res1.error) {

        fprintf(stderr, "Failed to find a root.\n");
        return EXIT_FAILURE;
    }
    printf("Root #1 of p(z) = z : ");
    printf("%f + %fi\n", creal(res1.value), cimag(res1.value));

    Newton_Root_Result res2 = find_root(poly2, -1, 1e-6);
    if (res2.error) {

        fprintf(stderr, "Failed to find a root.\n");
        return EXIT_FAILURE;
    }
    printf("Root #1 of p(z) = z^3 - 2z^2 + 5z + 4 : ");
    printf("%f + %fi\n", creal(res2.value), cimag(res2.value));

    Newton_Root_Result res3 = find_root(poly2, 1.3 + 2.2*I, 1e-6);
    if (res3.error) {

        fprintf(stderr, "Failed to find a root.\n");
        return EXIT_FAILURE;
    }
    printf("Root #2 of p(z) = z^3 - 2z^2 + 5z + 4 : ");
    printf("%f + %fi\n", creal(res3.value), cimag(res3.value));

    Newton_Root_Result res4 = find_root(poly2, 1.3 - 2.2*I, 1e-6);
    if (res4.error) {

        fprintf(stderr, "Failed to find a root.\n");
        return EXIT_FAILURE;
    }
    printf("Root #3 of p(z) = z^3 - 2z^2 + 5z + 4 : ");
    printf("%f + %fi\n", creal(res4.value), cimag(res4.value));

    return EXIT_SUCCESS;
}
