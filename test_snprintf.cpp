#include <stdio.h>
int main() {
    char search[32];
    snprintf(search, sizeof(search), "%s(", "6.8");
    printf("%s\n", search);
    return 0;
}
