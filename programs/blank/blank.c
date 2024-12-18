#include "peachos.h"

int main (int argc, char *argv[]) {
    print("Hello, how are you?\n");

    while(1) {
        if (getkey() != 0x0) {
            print("key was pressed");
        }
    }
    return 0x0;
}