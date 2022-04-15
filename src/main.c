/*
    utf8file - check if given file(s) is properly encoded in UTF-8.
    Copyright (C) 2022 Oleg<Ease>Kharchuk
    License GNU AGPLv3
*/
#include <stdio.h>
#include <stdlib.h>

/*
    _____________________________________
    |1st byte|2nd byte|3rd byte|4th byte|
    |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
    | Valid UTF-8 hex sequences RFC3629 |
    |___________________________________|
    | 00..7F |   --   |   --   |   --   |
    | C2..DF | 80..BF |   --   |   --   |
    |   E0   | A0..BF | 80..BF |   --   |
    | E1..EC | 80..BF | 80..BF |   --   |
    |   ED   | 80..9F | 80..BF |   --   |
    | EE..EF | 80..BF | 80..BF |   --   |
    |   F0   | 90..BF | 80..BF | 80..BF |
    | F1..F3 | 80..BF | 80..BF | 80..BF |
    |   F4   | 80..8F | 80..BF | 80..BF |
    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("No arguments, nothing to do.\n");
        return EXIT_SUCCESS;
    }

    char **files = argv;
    while (*++files != NULL) {
        char *filename = *files;
        printf("%s\n", filename);
    }

    return EXIT_SUCCESS;
}
