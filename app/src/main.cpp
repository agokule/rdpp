#include <iostream>
#include <format>
#include <string>

int main() {
    std::string message = std::format("The answer is {}", 42);
    std::cout << message << std::endl;
}
