#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/standard/info.h"
#include "Zend/zend_exceptions.h"

#include "php_libdeflate.h"
#include "libdeflate.h"


#define MAX_COMPRESSION_LEVEL 12
#define COMPRESSOR_CACHE_SIZE (MAX_COMPRESSION_LEVEL + 1) //extra slot for level 0

ZEND_BEGIN_MODULE_GLOBALS(libdeflate)
	struct libdeflate_compressor* compressor_cache[COMPRESSOR_CACHE_SIZE];
	struct libdeflate_decompressor* decompressor;
ZEND_END_MODULE_GLOBALS(libdeflate);

#define LIBDEFLATE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(libdeflate, v)

ZEND_DECLARE_MODULE_GLOBALS(libdeflate);

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(libdeflate)
{
#if defined(ZTS) && defined(COMPILE_DL_LIBDEFLATE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	memset(LIBDEFLATE_G(compressor_cache), 0, sizeof(struct libdeflate_compressor*) * COMPRESSOR_CACHE_SIZE);
	LIBDEFLATE_G(decompressor) = NULL;
	return SUCCESS;
}
/* }}} */

/* {{{ */
PHP_RSHUTDOWN_FUNCTION(libdeflate) {
	if (LIBDEFLATE_G(decompressor) != NULL) {
		libdeflate_free_decompressor(LIBDEFLATE_G(decompressor));
		LIBDEFLATE_G(decompressor) = NULL;
	}
	for (int i = 0; i < COMPRESSOR_CACHE_SIZE; i++) {
		struct libdeflate_compressor* compressor = LIBDEFLATE_G(compressor_cache)[i];
		if (compressor != NULL) {
			libdeflate_free_compressor(compressor);
		}
	}

	return SUCCESS;
} /* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(libdeflate)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "libdeflate support", "enabled");
	php_info_print_table_header(2, "libdeflate library version", LIBDEFLATE_VERSION_STRING);
	php_info_print_table_end();
}
/* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_libdeflate_compress, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

typedef size_t (*php_libdeflate_compress_bound_func)(struct libdeflate_compressor*, size_t);
typedef size_t (*php_libdeflate_compress_func)(struct libdeflate_compressor*, const void*, size_t, void*, size_t);

static inline zend_string* php_libdeflate_compress(zend_string *data, zend_long level, php_libdeflate_compress_bound_func compressBoundFunc, php_libdeflate_compress_func compressFunc) {
	if (level < 0 || level > MAX_COMPRESSION_LEVEL) {
		zend_throw_exception_ex(
			spl_ce_InvalidArgumentException,
			0,
			"Invalid compression level: %zi (accepted levels: %u...%u)",
			level,
			0,
			MAX_COMPRESSION_LEVEL
		);
		return NULL;
	}

	struct libdeflate_compressor* compressor = LIBDEFLATE_G(compressor_cache)[level];
	if (compressor == NULL) {
		compressor = libdeflate_alloc_compressor(level);
		if (compressor == NULL) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Unable to allocate libdeflate compressor (this is a bug)");
			return NULL;
		}
		LIBDEFLATE_G(compressor_cache)[level] = compressor;
	}

	size_t compressBound = compressBoundFunc(compressor, ZSTR_LEN(data));
	void* output = emalloc(compressBound);
	size_t actualSize = compressFunc(compressor, ZSTR_VAL(data), ZSTR_LEN(data), output, compressBound);

	if (actualSize == 0){
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Too small buffer provided (this is a bug)");
		return NULL;
	}

	zend_string* result = zend_string_init(output, actualSize, 0);
	efree(output);
	return result;
}

// php7 warns rather that throws with ZEND_PARSE_PARAMETERS_START
#if PHP_VERSION_ID >= 80000
#define LIBDEEFLATE_PARSE_PARAMS() ZEND_PARSE_PARAMETERS_START(1, 2)
#else
#define LIBDEEFLATE_PARSE_PARAMS() ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
#endif

#define PHP_LIBDEFLATE_FUNC(compressFunc, compressBoundFunc) \
PHP_FUNCTION(compressFunc) { \
	zend_string *data; \
	zend_long level = 6; \
\
	LIBDEEFLATE_PARSE_PARAMS() \
		Z_PARAM_STR(data) \
		Z_PARAM_OPTIONAL \
		Z_PARAM_LONG(level) \
	ZEND_PARSE_PARAMETERS_END(); \
\
	zend_string *result = php_libdeflate_compress(data, level, compressBoundFunc, compressFunc); \
	if (result == NULL) { \
		return; \
	} \
	RETURN_STR(result); \
}

/* {{{ proto string libdeflate_deflate_compress(string $data [, int $level = 6]) */
PHP_LIBDEFLATE_FUNC(libdeflate_deflate_compress, libdeflate_deflate_compress_bound) /* }}} */

/* {{{ proto string libdeflate_zlib_compress(string $data [, int $level = 6]) */
PHP_LIBDEFLATE_FUNC(libdeflate_zlib_compress, libdeflate_zlib_compress_bound) /* }}} */

/* {{{ proto string libdeflate_gzip_compress(string $data [, int $level = 6]) */
PHP_LIBDEFLATE_FUNC(libdeflate_gzip_compress, libdeflate_gzip_compress_bound) /* }}} */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_libdeflate_decompress, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, max_output_size, IS_LONG, 0)
ZEND_END_ARG_INFO()

typedef enum libdeflate_result (*php_libdeflate_decompress_func)(
	struct libdeflate_decompressor*,
	const void*,
	size_t,
	void*,
	size_t,
	size_t*
);

static inline zend_string* php_libdeflate_decompress(
	zend_string *data,
	zend_long max_output_size,
	php_libdeflate_decompress_func decompressFunc,
	bool isGzip
) {
	if (ZSTR_LEN(data) == 0) {
		return zend_empty_string;
	}

	struct libdeflate_decompressor* decompressor = LIBDEFLATE_G(decompressor);
	if (decompressor == NULL) {
		decompressor = libdeflate_alloc_decompressor();
		if (decompressor == NULL) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Unable to allocate libdeflate decompressor (this is a bug)");
			return NULL;
		}
		LIBDEFLATE_G(decompressor) = decompressor;
	}

	size_t max_limit = (max_output_size > 0) ? (size_t)max_output_size : (size_t)(1024 * 1024 * 256); // 256MB default ceiling

	size_t guess = 0;
	if (isGzip && ZSTR_LEN(data) >= 18) {
		// GZIP footer contains original ISIZE in the last 4 bytes (little-endian)
		const unsigned char* p = (const unsigned char*)ZSTR_VAL(data) + ZSTR_LEN(data) - 4;
		uint32_t isize = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
		if (isize > 0 && isize <= max_limit) {
			guess = isize;
		}
	}

	if (guess == 0) {
		if (max_output_size > 0 && (size_t)max_output_size <= 65536) {
			guess = (size_t)max_output_size;
		} else {
			guess = ZSTR_LEN(data) * 4;
			if (guess < 4096) guess = 4096;
			if (guess > max_limit) guess = max_limit;
		}
	}

	while (1) {
		zend_string* result = zend_string_alloc(guess, 0);
		size_t actualSize = 0;
		enum libdeflate_result res = decompressFunc(
			decompressor,
			ZSTR_VAL(data),
			ZSTR_LEN(data),
			ZSTR_VAL(result),
			guess,
			&actualSize
		);

		if (res == LIBDEFLATE_SUCCESS) {
			if (actualSize < guess) {
				result = zend_string_truncate(result, actualSize, 0);
			}
			ZSTR_VAL(result)[actualSize] = '\0';
			return result;
		}

		zend_string_release(result);

		if (res == LIBDEFLATE_INSUFFICIENT_SPACE) {
			if (guess >= max_limit) {
				zend_throw_exception_ex(
					spl_ce_RuntimeException,
					0,
					"Decompressed size exceeds maximum allowed buffer limit of %zu bytes",
					max_limit
				);
				return NULL;
			}
			if (guess > max_limit / 2) {
				guess = max_limit;
			} else {
				guess *= 2;
			}
			continue;
		}

		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Corrupt or invalid compressed data");
		return NULL;
	}
}

#define PHP_LIBDEFLATE_DECOMPRESS_FUNC(funcName, decompressFunc, isGzip) \
PHP_FUNCTION(funcName) { \
	zend_string *data; \
	zend_long max_output_size = 0; \
\
	LIBDEEFLATE_PARSE_PARAMS() \
		Z_PARAM_STR(data) \
		Z_PARAM_OPTIONAL \
		Z_PARAM_LONG(max_output_size) \
	ZEND_PARSE_PARAMETERS_END(); \
\
	zend_string *result = php_libdeflate_decompress(data, max_output_size, decompressFunc, isGzip); \
	if (result == NULL) { \
		return; \
	} \
	RETURN_STR(result); \
}

/* {{{ proto string libdeflate_deflate_decompress(string $data [, int $max_output_size = 0]) */
PHP_LIBDEFLATE_DECOMPRESS_FUNC(libdeflate_deflate_decompress, libdeflate_deflate_decompress, false) /* }}} */

/* {{{ proto string libdeflate_zlib_decompress(string $data [, int $max_output_size = 0]) */
PHP_LIBDEFLATE_DECOMPRESS_FUNC(libdeflate_zlib_decompress, libdeflate_zlib_decompress, false) /* }}} */

/* {{{ proto string libdeflate_gzip_decompress(string $data [, int $max_output_size = 0]) */
PHP_LIBDEFLATE_DECOMPRESS_FUNC(libdeflate_gzip_decompress, libdeflate_gzip_decompress, true) /* }}} */

/* {{{ libdeflate_functions[]
 */
static const zend_function_entry libdeflate_functions[] = {
	PHP_FE(libdeflate_deflate_compress, arginfo_libdeflate_compress)
	PHP_FE(libdeflate_zlib_compress, arginfo_libdeflate_compress)
	PHP_FE(libdeflate_gzip_compress, arginfo_libdeflate_compress)
	PHP_FE(libdeflate_deflate_decompress, arginfo_libdeflate_decompress)
	PHP_FE(libdeflate_zlib_decompress, arginfo_libdeflate_decompress)
	PHP_FE(libdeflate_gzip_decompress, arginfo_libdeflate_decompress)
	PHP_FE_END
};
/* }}} */

/* {{{ libdeflate_module_entry
 */
zend_module_entry libdeflate_module_entry = {
	STANDARD_MODULE_HEADER,
	"libdeflate",
	libdeflate_functions,
	NULL,
	NULL, /* PHP_MSHUTDOWN */
	PHP_RINIT(libdeflate),
	PHP_RSHUTDOWN(libdeflate),
	PHP_MINFO(libdeflate),
	PHP_LIBDEFLATE_VERSION,
	PHP_MODULE_GLOBALS(libdeflate),
	NULL,
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_LIBDEFLATE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(libdeflate)
#endif
