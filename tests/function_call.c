int add(int left, int right) {
    return left + right;
}

int sum_down(int value) {
    if (value <= 0) {
        return 0;
    }
    return value + sum_down(value - 1);
}

int main() {
    int total = add(8, 5) + sum_down(3);
    return total;
}
