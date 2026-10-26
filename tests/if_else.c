int main() {
    int value = 4;
    if (value >= 4 && !(value == 0)) {
        if (value < 5) {
            value = value + 1;
        } else {
            value = value * 2;
        }
    } else {
        value = value - 1;
    }
    return value;
}
