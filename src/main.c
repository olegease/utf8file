/*
    utf8file - check if given file(s) is properly encoded in UTF-8.
    Copyright (C) 2022 Oleg<Ease>Kharchuk
    License GNU AGPLv3
*/
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_LAST 4095
#define BUFFER_SIZE (BUFFER_LAST + 1)

#define EXPECT_ERROR 0
#define EXPECT_DEFAULT 1
#define EXPECT_CONTINUATION 2
#define EXPECT_CONTINUATION_CHUNK 3

typedef unsigned char BufferElement;
typedef int (*expectFunction) (BufferElement c, int* cnt, int* beg, int* end);

int expectError(BufferElement c, int* cnt, int* beg, int* end);
int expectDefault(BufferElement c, int* cnt, int* beg, int* end);
int expectContinuation(BufferElement c, int* cnt, int* beg, int* end);
int expectContinuationChunk(BufferElement c, int* cnt, int* beg, int* end);

expectFunction Expects[] = {
    expectError,
    expectDefault,
    expectContinuation,
    expectContinuationChunk
};

BufferElement Buffer[BUFFER_SIZE];

/*
    _____________________________________
    |1st byte|2nd byte|3rd byte|4th byte|
    |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
    | Valid UTF-8 hex sequences RFC3629 |
    |___________________________________|
    | 00..7F |   --   |   --   |   --   |
    | C2..DF | 80..BF |   --   |   --   |
    |   E0   |▼A0..BF | 80..BF |   --   |
    | E1..EC | 80..BF | 80..BF |   --   |
    |   ED   | 80..9F▼| 80..BF |   --   |
    | EE..EF | 80..BF | 80..BF |   --   |
    |   F0   |▼90..BF | 80..BF | 80..BF |
    | F1..F3 | 80..BF | 80..BF | 80..BF |
    |   F4   | 80..8F▼| 80..BF | 80..BF |
    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
*/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("No arguments, nothing to do.\n");
        return EXIT_SUCCESS;
    }

    char **fileArgs = argv;
    FILE *f = NULL;
    int next = EXPECT_DEFAULT, cnt = 0, beg = 0, end = 0;
    while (*++fileArgs != NULL) {
        char *fileName = *fileArgs;
        printf("Start processing file: %s\n", fileName);

        f = fopen(fileName, "rb");
        if (f == NULL) {
            printf("Failed to open file: %s\n", fileName);
            continue;
        }

        size_t rSize = BUFFER_LAST;
        while (rSize == BUFFER_LAST) {
            rSize = fread(Buffer, sizeof(BufferElement), BUFFER_LAST, f);
            int i = 0;
            while ((next = Expects[next](Buffer[i], &cnt, &beg, &end)) && ++i != rSize) {}
            if (next == EXPECT_ERROR) break;
        }

        if (next != EXPECT_DEFAULT) {
            printf("Error not UTF-8 file: %s\n", fileName);
        } else if (feof(f)) {
            printf("Success UTF-8 properly encoded file: %s\n", fileName);
        } else if (ferror(f)) {
            printf("Failed to read file: %s\n", fileName);
        } else {
            printf("Cannot reach end of file: %s\n", fileName);
        }

        fclose(f);
    }

    return EXIT_SUCCESS;
}

int expectError(BufferElement c, int* cnt, int* beg, int* end)
{
    return EXPECT_DEFAULT;
}
int expectDefault(BufferElement c, int* cnt, int* beg, int* end)
{
    if (c <= 0x7F) return EXPECT_DEFAULT;
    if (c >= 0xC2 && c <= 0xDF) {
        *cnt = 1;
        return EXPECT_CONTINUATION;
    }
    if (c >= 0xE1 && c <= 0xEF && c != 0xED) {
        *cnt = 2;
        return EXPECT_CONTINUATION;
    }
    if (c >= 0xF1 && c <= 0xF3) {
        *cnt = 3;
        return EXPECT_CONTINUATION;
    }
    switch (c) {
        case 0xE0: {
            *cnt = 1; *beg = 0xA0; *end = 0xBF;
            return EXPECT_CONTINUATION_CHUNK;
        }
        case 0xED: {
            *cnt = 1; *beg = 0x80; *end = 0x9F;
            return EXPECT_CONTINUATION_CHUNK;
        }
        case 0xF0: {
            *cnt = 2; *beg = 0x90; *end = 0xBF;
            return EXPECT_CONTINUATION_CHUNK;
        }
        case 0xF4: {
            *cnt = 2; *beg = 0x90; *end = 0xBF;
            return EXPECT_CONTINUATION_CHUNK;
        }
        default: return EXPECT_ERROR;
    }
}
int expectContinuation(BufferElement c, int* cnt, int* beg, int* end)
{
    if (c >= 0x80 && c <= 0xBF) {
        *cnt -= 1;
        return *cnt ? EXPECT_CONTINUATION : EXPECT_DEFAULT;
    }
    return EXPECT_ERROR;
}
int expectContinuationChunk(BufferElement c, int* cnt, int* beg, int* end)
{
    return (c >= *beg && c <= *end) ? EXPECT_CONTINUATION : EXPECT_ERROR;
}
