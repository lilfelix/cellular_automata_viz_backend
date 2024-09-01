#include <iostream>


int generate_int_rules(int n) {
    for (int i = 0; i < n; i++) {
        std::cout << i << std::endl;
    }
    return 0;
}

int main() {
    generate_int_rules(10);
    return 0;
}