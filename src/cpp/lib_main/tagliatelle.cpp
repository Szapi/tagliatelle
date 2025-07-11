#include "tagliatelle.h"

extern "C" {
    int tagliatelle_double_value(int value) {
        return value * 2;
    }

    const char* tagliatelle_get_version(void) {
        return "1.0.0";
    }
}
