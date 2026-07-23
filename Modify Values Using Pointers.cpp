#include <iostream>

int main() {
    // 1. Create an integer variable with value 10
    int num = 10;

    // Display the original value
    std::cout << "Original Value: " << num << std::endl;

    // 2. Create a pointer to the variable
    int* ptr = &num;

    // 3. Change the value to 50 using the pointer
    *ptr = 50;

    // 4. Display the updated value
    std::cout << "Updated Value: " << num << std::endl;

    return 0;
}