
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 5;
    int y = add(x, 3);

    if (y > 5) {
        y = y * 2;
    } else {
        y = y - 1;
    }

    while (x > 0) {
        x = x - 1;
    }

    return y;
}

