#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum Num_Deriv_Error{
    
    SUCCESS = 0,
    UNDEFINED = 1,
    UNSTABLE = 2,
};

typedef struct Num_Deriv_Result Num_Deriv_Result;
struct Num_Deriv_Result{
    
    double value;
    enum Num_Deriv_Error error;
};


Num_Deriv_Result 
num_deriv(double(*f)(double), const double x, const double dx) {
    
    const double h = fabs(dx/2);
    const double f1 = f(x - 2*h);
    const double f2 = f(x - h);
    const double f3 = f(x);
    const double f4 = f(x + h);
    const double f5 = f(x + 2*h);
    
    Num_Deriv_Result result = {0, SUCCESS};

    if (fpclassify(f1) == FP_NAN || fpclassify(f1) == FP_INFINITE ||
        fpclassify(f2) == FP_NAN || fpclassify(f2) == FP_INFINITE ||
        fpclassify(f3) == FP_NAN || fpclassify(f3) == FP_INFINITE ||
        fpclassify(f4) == FP_NAN || fpclassify(f4) == FP_INFINITE ||
        fpclassify(f5) == FP_NAN || fpclassify(f5) == FP_INFINITE) {

        result.error = UNDEFINED;
        return result;
    }

    const double dfdx = (f1 - 8*f2 + 8*f4 - f5) / (12*h);
    result.value = dfdx;
    
    if (fpclassify(dfdx) == FP_NAN || fpclassify(dfdx) == FP_INFINITE) {

        result.error = UNDEFINED;
        return result;
    }
    
    const bool pos_monotonic = (f1 < f2 && f2 < f3 && f3 < f4 && f4 < f5);
    const bool neg_monotonic = (f1 > f2 && f2 > f3 && f3 > f4 && f4 > f5);

    if (pos_monotonic || neg_monotonic) { 
        return result;
    }
    
    const bool right_hump = (f3 > f3 && f4 > f5) || (f4 < f3 && f4 < f5);
    const bool left_hump = (f2 > f1 && f2 > f3) || (f2 < f1 && f2 < f3); 

    if (right_hump) {
        
        if (left_hump || fabs(2*f4 - f3 - f5) > fabs(f3 - f1)) { 
            
            result.error = UNSTABLE;
            return result;
        }
    }

    if (left_hump) {
        
        if (right_hump || fabs(2*f2 - f1 - f3) > fabs(f5 - f3)) {
            
            result.error = UNSTABLE;
            return result;
        }
        
    }
    
    return result;
}


double
testf1(const double x) {
    
    return 2*x;
}


double
testf2(const double x) {

    return x*x;
}


double
testf3(const double x) {

    return cos(1.0/x);
}


double
testf4(const double x) {

    return pow(x, 0.333);
}


int
main() {

    const double EPS = 1e-9;
    Num_Deriv_Result res;
    const char * err_txt[] = {"Solution is valid.",
                              "Derivative does not exist.",
                              "Derivative is unstable."};

    res = num_deriv(testf1, 2.0, EPS);
    if (!res.error) {
        printf("d/dx (x) @ 2.0 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }
    
    res = num_deriv(testf2, 0.0, EPS);
    if (!res.error) {
        printf("d/dx (x^2) @ 0.0 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }

    res = num_deriv(testf2, -1.0, EPS);
    if (!res.error) {
        printf("d/dx (x^2) @ -1.0 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }

    res = num_deriv(testf2, 5.0e-10, EPS);
    if (!res.error) {
        printf("d/dx (x^2) @ 5.0e-10 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }
    
    res = num_deriv(testf3, 3.0e-9, EPS);
    if (!res.error) {
        printf("d/dx (cos(1/x)) @ 3.0e-9 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }
    
    res = num_deriv(testf3, 1.0/3.141592654, EPS);
    if (!res.error) {
        printf("d/dx (cos(1/x)) @ 1/pi = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }

    res = num_deriv(testf4, 0.0, EPS);
    if (!res.error) {
        printf("d/dx (x^0.333) @ 0.0 = %f\n", res.value);
    } else {
        printf("Cannot compute the derivative! %s\n", err_txt[res.error]);
    }

    return EXIT_SUCCESS;
}

