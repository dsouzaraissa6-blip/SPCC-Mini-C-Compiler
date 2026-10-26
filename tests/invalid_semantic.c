int add(int left, int right) {
    return left + right;
}

int main() {
    int result = missing(1);
    result = undeclared_value + add(2);
    return result;
}
