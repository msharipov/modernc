// Computes the complex derivative of an analytic function

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>

typedef double complex lfc_t;

enum Num_Deriv_Error{
    
    SUCCESS = 0,
    UNDEFINED = 1,
    UNSTABLE = 2,
};


struct Cmpl_Deriv_Result{

    lfc_t value;
    enum Num_Deriv_Error error;
};


// Given z = x + iy and f(z) = u(x,y) + iv(x,y), computes f'(z) as:
// du/dx + i*dv/dx with both derivatives evaluated at z_0
struct Cmpl_Deriv_Result 
cmpl_deriv(lfc_t(*f)(const lfc_t), const lfc_t z, const double dz) {
    
    const double h = fabs(dz/2);

    const lfc_t f1 = f(z - 2*(lfc_t)h);
    const lfc_t f2 = f(z - (lfc_t)h);
    const lfc_t f3 = f(z);
    const lfc_t f4 = f(z + (lfc_t)h);
    const lfc_t f5 = f(z + 2*(lfc_t)h);

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

    struct Cmpl_Deriv_Result result = {0, SUCCESS};
    
    const double dudx = (u1 - 8*u2 + 8*u4 - u5) / (12*h);
    const double dvdx = (v1 - 8*v2 + 8*v4 - v5) / (12*h);

    if (fpclassify(dudx) == FP_NAN || fpclassify(dudx) == FP_INFINITE ||
        fpclassify(dvdx) == FP_NAN || fpclassify(dvdx) == FP_INFINITE ||
        fpclassify(u3) == FP_NAN || fpclassify(u3) == FP_INFINITE ||
        fpclassify(v3) == FP_NAN || fpclassify(v3) == FP_INFINITE) {

        result.error = UNDEFINED;
        return result;
    }
    
    result.value = (lfc_t)dudx + I*(lfc_t)dvdx;

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


lfc_t
func1(const lfc_t z) {
    
    return z;
}


lfc_t
func2(const lfc_t z) {
    
    return z*z;
}


lfc_t
func3(const lfc_t z) {
    
    return ccos(1/z)*z;
}


lfc_t
func4(const lfc_t z) {

    return 1/z;
}


int
main() {
    
    struct Cmpl_Deriv_Result res;

    res = cmpl_deriv(func1, 1 + I*3, 1e-6);
    printf("Test #1: ");
    switch (res.error) {
        case UNDEFINED:
            printf("Derivative is not defined!\n");
            break;
        case UNSTABLE:
            printf("Derivative is unstable!\n");
            break;
        case SUCCESS:
            printf("%f + %fi\n", creal(res.value), cimag(res.value));
            break;
        default:
            printf("Unknown error!\n");
    }

    res = cmpl_deriv(csin, I, 1e-6);
    printf("Test #2: ");
    switch (res.error) {
        case UNDEFINED:
            printf("Derivative is not defined!\n");
            break;
        case UNSTABLE:
            printf("Derivative is unstable!\n");
            break;
        case SUCCESS:
            printf("%f + %fi\n", creal(res.value), cimag(res.value));
            break;
        default:
            printf("Unknown error!\n");
    }

    res = cmpl_deriv(func2, -1 + 2*I, 1e-6);
    printf("Test #3: ");
    switch (res.error) {
        case UNDEFINED:
            printf("Derivative is not defined!\n");
            break;
        case UNSTABLE:
            printf("Derivative is unstable!\n");
            break;
        case SUCCESS:
            printf("%f + %fi\n", creal(res.value), cimag(res.value));
            break;
        default:
            printf("Unknown error!\n");
    }

    res = cmpl_deriv(func4, 0, 1e-6);
    printf("Test #4: ");
    switch (res.error) {
        case UNDEFINED:
            printf("Derivative is not defined!\n");
            break;
        case UNSTABLE:
            printf("Derivative is unstable!\n");
            break;
        case SUCCESS:
            printf("%f + %fi\n", creal(res.value), cimag(res.value));
            break;
        default:
            printf("Unknown error!\n");
    }

    res = cmpl_deriv(func4, 7e-7, 1e-6);
    printf("Test #5: ");
    switch (res.error) {
        case UNDEFINED:
            printf("Derivative is not defined!\n");
            break;
        case UNSTABLE:
            printf("Derivative is unstable!\n");
            break;
        case SUCCESS:
            printf("%f + %fi\n", creal(res.value), cimag(res.value));
            break;
        default:
            printf("Unknown error!\n");
    }

    exit(EXIT_SUCCESS);
}
