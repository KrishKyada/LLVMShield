/*
 * example.c - Educational Obfuscation Demo Program
 *
 * This program demonstrates various constructs that will be obfuscated
 * by the warp_aai toolchain, including:
 * - String constants (for XOR encryption)
 * - Functions (for bogus function insertion and control flow changes)
 * - Static/private globals (for symbol renaming)
 *
 * EDUCATIONAL PURPOSE: This is a simple test program to verify that
 * the obfuscation techniques are working correctly.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Global string constants - these will be XOR encrypted
static const char* secret_message = "This is a secret message that should be obfuscated!";
static const char* app_name = "warp_aai Educational Obfuscation Demo";
static const char* version_info = "Version 1.0.0 - Educational MVP";

// Static variables - these will be renamed with _obf suffix
static int global_counter = 0;
static double calculation_result = 0.0;

// Function prototypes
int calculate_fibonacci(int n);
void print_banner(void);
void demonstrate_strings(void);
int perform_calculations(int base_value);

/**
 * Calculate Fibonacci number (recursive implementation)
 * This function may have control flow obfuscation applied
 */
int calculate_fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return calculate_fibonacci(n - 1) + calculate_fibonacci(n - 2);
}

/**
 * Print application banner with obfuscated strings
 */
void print_banner(void) {
    printf("==========================================\n");
    printf("%s\n", app_name);
    printf("%s\n", version_info);
    printf("==========================================\n\n");
}

/**
 * Demonstrate string usage that will be obfuscated
 */
void demonstrate_strings(void) {
    printf("Demonstrating string obfuscation:\n");
    printf("Secret: %s\n", secret_message);
    
    // Local string that might also be obfuscated
    const char* local_message = "This is a local string constant";
    printf("Local message: %s\n", local_message);
    
    // String manipulation
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Processed message length: %zu characters", 
             strlen(secret_message));
    printf("%s\n\n", buffer);
}

/**
 * Perform some calculations to demonstrate function obfuscation
 */
int perform_calculations(int base_value) {
    global_counter++;
    
    // Simple arithmetic
    int result = base_value * 2 + 10;
    calculation_result = (double)result / 3.14159;
    
    printf("Calculation %d: %d * 2 + 10 = %d\n", global_counter, base_value, result);
    printf("Floating point result: %.2f\n", calculation_result);
    
    return result;
}

/**
 * Main function - entry point for the demo program
 */
int main(int argc, char* argv[]) {
    printf("Starting warp_aai obfuscation demonstration...\n\n");
    
    // Print banner with obfuscated strings
    print_banner();
    
    // Demonstrate string obfuscation
    demonstrate_strings();
    
    // Perform calculations with different values
    printf("Running calculations (functions may have bogus code inserted):\n");
    for (int i = 1; i <= 5; i++) {
        int calc_result = perform_calculations(i * 7);
        printf("Result for iteration %d: %d\n", i, calc_result);
    }
    printf("\n");
    
    // Calculate and display Fibonacci numbers
    printf("Fibonacci sequence (function may have control flow obfuscation):\n");
    for (int i = 0; i < 10; i++) {
        int fib = calculate_fibonacci(i);
        printf("fib(%d) = %d\n", i, fib);
    }
    printf("\n");
    
    // Final statistics
    printf("Program execution summary:\n");
    printf("- Total calculations performed: %d\n", global_counter);
    printf("- Last calculation result: %.2f\n", calculation_result);
    printf("- Command line arguments: %d\n", argc);
    
    if (argc > 1) {
        printf("- First argument: %s\n", argv[1]);
    }
    
    printf("\nObfuscation techniques that were applied:\n");
    printf("1. XOR encryption of string constants\n");
    printf("2. Insertion of bogus/fake functions\n");
    printf("3. Renaming of private global symbols\n");
    printf("4. Dead conditional branch insertion\n");
    
    printf("\nEducational demo completed successfully!\n");
    printf("Check the generated report for obfuscation statistics.\n");
    
    return 0;
}