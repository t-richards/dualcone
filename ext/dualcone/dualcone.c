#include "dualcone.h"

VALUE rb_mDualcone;

void rb_dualcone_cleanup(DualconeContext *ctx) {
  if (ctx->input_path != NULL) {
    free(ctx->input_path);
  }

  if (ctx->output_path != NULL) {
    unlink(ctx->output_path); /* temporary file */
    hydro_memzero(ctx->output_path, PATH_MAX);
    free(ctx->output_path);
  }

  if (ctx->plaintext != NULL) {
    hydro_memzero(ctx->plaintext, ctx->plaintext_len);
    free(ctx->plaintext);
  }

  if (ctx->ciphertext_hex != NULL) {
    hydro_memzero(ctx->ciphertext_hex, ctx->ciphertext_hex_len);
    free(ctx->ciphertext_hex);
  }

  if (ctx->ciphertext != NULL) {
    hydro_memzero(ctx->ciphertext, ctx->ciphertext_len);
    free(ctx->ciphertext);
  }

  hydro_memzero(ctx, sizeof(DualconeContext));
}

void rb_dualcone_get_key(DualconeContext *ctx) {
  int result = 0;
  int errno_sv = 0;

  /* Hex-encoded encryption key from environment */
  char *hex_key = getenv(DUALCONE_HEX_KEY);
  if (hex_key == NULL) {
    rb_dualcone_cleanup(ctx);
    rb_raise(rb_eKeyError, "environment variable not found: " DUALCONE_HEX_KEY);
  }

  /* Convert encryption key (hex encoded) to binary */
  result = hydro_hex2bin(ctx->binary_key, hydro_secretbox_KEYBYTES, hex_key, strlen(hex_key), NULL, NULL);
  if (result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(ctx);
    rb_raise(rb_eFatal, "unable to hex-decode encryption key: %s", strerror(errno_sv));
  }
}

VALUE rb_dualcone_run(VALUE _self, VALUE code) {
  int result = 0;
  int errno_sv = 0;

  /* Check args */
  rb_check_type(code, T_STRING);

  /* Dualcone private memory allocations */
  DualconeContext ctx = {0};
  rb_dualcone_get_key(&ctx);

  /* Encrypted ruby code (hex-encoded) */
  char *hex_code = StringValuePtr(code);
  long hex_code_len = RSTRING_LEN(code);

  /* Check message length */
  if (hex_code_len < DUALCONE_MIN_HEX_LEN) {
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to run code: too short (got %ld chars, expected at least %d chars)", hex_code_len, DUALCONE_MIN_HEX_LEN);
  }

  /* Allocate memory for ciphertext */
  ctx.ciphertext_len = hex_code_len / 2;
  ctx.ciphertext = calloc(1, ctx.ciphertext_len);
  if (RB_UNLIKELY(ctx.ciphertext == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for ciphertext: %s", strerror(errno_sv));
  }

  /* Convert code (hex encoded) to binary */
  result = hydro_hex2bin(ctx.ciphertext, ctx.ciphertext_len, hex_code, hex_code_len, NULL, NULL);
  if (result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to hex-decode ruby code: %s", strerror(errno_sv));
  }

  /* Allocate memory for plaintext */
  ctx.plaintext_len = ctx.ciphertext_len - hydro_secretbox_HEADERBYTES;
  ctx.plaintext = calloc(1, ctx.plaintext_len);
  if (RB_UNLIKELY(ctx.plaintext == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for plaintext: %s", strerror(errno_sv));
  }

  /* Decrypt binary code to plaintext code */
  /* TODO(tom): Fix off-by-one error in encrypt? Null byte is not always present at end of string. */
  result = hydro_secretbox_decrypt(ctx.plaintext, ctx.ciphertext, ctx.ciphertext_len, 0, DUALCONE_CONTEXT, ctx.binary_key);
  if (result == -1) {
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to decrypt ruby code");
  }

  /* Evaluate the plaintext code */
  rb_eval_string_protect(ctx.plaintext, &result);
  if (result != 0) {
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to evaluate ruby code");
  }

  /* Done */
  rb_dualcone_cleanup(&ctx);
  return Qnil;
}

VALUE rb_dualcone_generate_key(VALUE _self) {
  uint8_t key[hydro_secretbox_KEYBYTES];
  char hex[hydro_secretbox_KEYBYTES * 2 + 1];

  /* Generate key */
  hydro_secretbox_keygen(key);
  char *retval = hydro_bin2hex(hex, hydro_secretbox_KEYBYTES * 2 + 1, key, hydro_secretbox_KEYBYTES);
  if (retval == NULL) {
    rb_raise(rb_eFatal, "unable to generate key");
  }

  return rb_str_new_cstr(hex);
}

VALUE rb_dualcone_encrypt(VALUE _self, VALUE path) {
  int result = 0;
  int errno_sv = 0;

  /* Check args */
  rb_check_type(path, T_STRING);

  /* Dualcone private memory allocations */
  DualconeContext ctx = {0};
  rb_dualcone_get_key(&ctx);

  /* The path of the input file (ruby code, plaintext) */
  /* This value is modified by dirname(), so strdup() is necessary */
  ctx.input_path = strdup(StringValuePtr(path));
  if (RB_UNLIKELY(ctx.input_path == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for input path: %s", strerror(errno_sv));
  }

  /* Check if input file is readable */
  int plaintext_fd = open(ctx.input_path, O_RDONLY);
  if (plaintext_fd == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to read input file '%s': %s", ctx.input_path, strerror(errno_sv));
  }

  /* Allocate memory for temporary path template */
  ctx.output_path = calloc(1, PATH_MAX);
  if (RB_UNLIKELY(ctx.output_path == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for output path: %s", strerror(errno_sv));
  }

  /* Construct path template for temporary file */
  char *output_dir = dirname(ctx.input_path);
  strncat(ctx.output_path, output_dir, PATH_MAX - strlen(ctx.output_path) - 1);
  strncat(ctx.output_path, "/.dualcone.XXXXXX", PATH_MAX - strlen(ctx.output_path) - 1);

  /* Create temporary file */
  int output_fd = mkstemp(ctx.output_path);
  if (RB_UNLIKELY(output_fd == -1)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to create temporary file: %s", strerror(errno_sv));
  }

  /* Get input file length */
  struct stat plaintext_stat = {0};
  result = fstat(plaintext_fd, &plaintext_stat);
  if (result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to determine length of input file: %s", strerror(errno_sv));
  }

  /* Allocate buffer for input file */
  ctx.plaintext_len = plaintext_stat.st_size;
  ctx.plaintext = calloc(1, ctx.plaintext_len);
  if (RB_UNLIKELY(ctx.plaintext == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for input data: %s", strerror(errno_sv));
  }

  /* Read entire input file */
  ssize_t read_result = read(plaintext_fd, ctx.plaintext, ctx.plaintext_len);
  if (read_result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to read ruby code: %s", strerror(errno_sv));
  }
  close(plaintext_fd);

  /* Allocate memory for encryption result */
  ctx.ciphertext_len = hydro_secretbox_HEADERBYTES + ctx.plaintext_len;
  ctx.ciphertext = calloc(1, ctx.ciphertext_len);
  if (RB_UNLIKELY(ctx.ciphertext == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for encryption: %s", strerror(errno_sv));
  }

  /* Encrypt data! */
  result = hydro_secretbox_encrypt(ctx.ciphertext, ctx.plaintext, ctx.plaintext_len, 0, DUALCONE_CONTEXT, ctx.binary_key);
  if (result != 0) {
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to encrypt ruby code");
  }

  /* Allocate memory for hex-encoded encrypted data */
  ctx.ciphertext_hex_len = ctx.ciphertext_len * 2 + 1;
  ctx.ciphertext_hex = calloc(1, ctx.ciphertext_hex_len);
  if (RB_UNLIKELY(ctx.ciphertext_hex == NULL)) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to allocate memory for hex-encoding: %s", strerror(errno_sv));
  }

  /* Hex encode encrypted data */
  hydro_bin2hex(ctx.ciphertext_hex, ctx.ciphertext_hex_len, ctx.ciphertext, ctx.ciphertext_len);

  /* Write preamble */
  ssize_t write_result = 0;
  write_result = write(output_fd, DUALCONE_PREAMBLE, sizeof(DUALCONE_PREAMBLE) - 1);
  if (write_result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to write to temporary file: %s", strerror(errno_sv));
  }

  /* Write hex-encoded data */
  write_result = write(output_fd, ctx.ciphertext_hex, ctx.ciphertext_hex_len - 1);
  if (write_result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to write to temporary file: %s", strerror(errno_sv));
  }

  /* Write postamble */
  write_result = write(output_fd, DUALCONE_POSTAMBLE, sizeof(DUALCONE_POSTAMBLE) - 1);
  if (write_result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to write to temporary file: %s", strerror(errno_sv));
  }

  /* Rename tempfile over original */
  close(output_fd);
  char *plaintext_path = StringValuePtr(path);
  result = rename(ctx.output_path, plaintext_path);
  if (result == -1) {
    errno_sv = errno;
    rb_dualcone_cleanup(&ctx);
    rb_raise(rb_eFatal, "unable to rename temporary file: %s", strerror(errno_sv));
  }

  /* Done */
  rb_dualcone_cleanup(&ctx);
  return Qnil;
}

void Init_dualcone(void) {
  if (RB_UNLIKELY(hydro_init() != 0)) {
    rb_raise(rb_eFatal, "unable to initialize libhydrogen");
    return;
  }

  rb_mDualcone = rb_define_module("Dualcone");
  rb_define_module_function(rb_mDualcone, "encrypt", rb_dualcone_encrypt, 1);
  rb_define_module_function(rb_mDualcone, "generate_key", rb_dualcone_generate_key, 0);
  rb_define_module_function(rb_mDualcone, "run", rb_dualcone_run, 1);
}
