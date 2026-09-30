#include "clib.h"

/* The stream table: stdin, stdout, stderr, stdaux and stdprn, then free
   slots for fopen. */

FILE __iob[20] = {
    { NULL, 0, NULL, _READ, 0, 0, 0, 0 },
    { NULL, 0, NULL, _WRITE, 1, 0, 0, 0 },
    { NULL, 0, NULL, _WRITE, 2, 0, 0, 0 },
    { NULL, 0, NULL, _READ | _WRITE, 3, 0, 0, 0 },
    { NULL, 0, NULL, _WRITE, 4, 0, 0, 0 },
};

char __tmpfnext = 'a';
int _fmode = 0x0100; /* O_TEXT */

/* Pulls in stinit, which opens and closes the streams. */
static __stream_link **force = &__OpenStreams;
