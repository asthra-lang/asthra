/**
 * Asthra Programming Language Runtime Entry Point
 * Provides the C main function that calls the Asthra _main
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

// Forward declaration of the Asthra main function
void _main(void);

// C entry point
int main(int argc, char *argv[]) {
    // Initialize runtime if needed
    
    // Call the Asthra main function
    _main();
    
    // Return success
    return 0;
}