#ifndef __DUALCONE_H
#define __DUALCONE_H

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ruby.h>
#include <hydrogen.h>

#define DUALCONE_CONTEXT "DUALCONE"
#define DUALCONE_HEX_KEY "DUALCONE_HEX_KEY"
#define DUALCONE_MIN_HEX_LEN hydro_secretbox_HEADERBYTES * 2
#define DUALCONE_PREAMBLE "require 'dualcone'\nDualcone.run('"
#define DUALCONE_POSTAMBLE "')\n"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    /* Symmetric key */
    uint8_t  binary_key[hydro_secretbox_KEYBYTES];

    /* Input file path */
    char    *input_path;

    /* Temporary output file path */
    char    *output_path;

    /* Encrypted code (hex-encoded) */
    char    *ciphertext_hex;
    size_t   ciphertext_hex_len;

    /* Encrypted code (binary) */
    uint8_t *ciphertext;
    size_t   ciphertext_len;

    /* Plaintext ruby code */
    char    *plaintext;
    size_t   plaintext_len;
} DualconeContext;

#endif /* __DUALCONE_H */
