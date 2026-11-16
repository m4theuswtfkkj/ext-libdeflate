# ext-libdeflate
[![CI](https://github.com/pmmp/ext-libdeflate/actions/workflows/ci.yml/badge.svg)](https://github.com/pmmp/ext-libdeflate/actions/workflows/ci.yml)

PHP bindings for [libdeflate](https://github.com/ebiggers/libdeflate), a zlib replacement with significantly better performance.

## Features
Upstream `ext-libdeflate` originally only exposed compression APIs due to libdeflate lacking a streaming decompression interface.

This fork extends the extension to expose **whole-buffer decompression APIs** for raw DEFLATE, ZLIB, and GZIP formats (`libdeflate_deflate_decompress`, `libdeflate_zlib_decompress`, and `libdeflate_gzip_decompress`):
- **Single-pass GZIP decompression:** Automatically inspects the 4-byte uncompressed size (`ISIZE`) in the GZIP footer to allocate the exact buffer size needed, avoiding any `realloc()` calls.
- **Dynamic buffer scaling:** For DEFLATE and ZLIB streams without known output size, an exponential buffer doubling strategy is used.
- **Zip-bomb protection:** An optional `$max_output_size` parameter (default 256 MB) prevents unbounded memory allocation on malicious payloads.

## Performance

### Compression
Compared to `zlib_encode()`, a 40% performance improvement can be observed at level 1, rising to 50-70% at level 6 (default), for approximately the same compression ratios. Also, `libdeflate_deflate_compress()` was observed to sometimes be faster at level 6 compression than `zlib_encode()` at level 1 compression.

Decompression is 2.2x-7.3x faster than `zlib_decode()` depending on format and payload size. Device specs and full tables: [docs/BENCHMARKS.md](docs/BENCHMARKS.md).

## API
```php
<?php

/**
 * Equivalent to zlib_encode($data, ZLIB_ENCODING_RAW, $level)
 * @throws \RuntimeException
 */
function libdeflate_deflate_compress(string $data, int $level = 6) : string;

/**
 * Equivalent to zlib_encode($data, ZLIB_ENCODING_DEFLATE, $level)
 * @throws \RuntimeException
 */
function libdeflate_zlib_compress(string $data, int $level = 6) : string;

/**
 * Equivalent to zlib_encode($data, ZLIB_ENCODING_GZIP, $level)
 * @throws \RuntimeException
 */
function libdeflate_gzip_compress(string $data, int $level = 6) : string;

/**
 * Equivalent to gzinflate($data, $max_output_size)
 * @param int $max_output_size Maximum allowed output size in bytes (0 = unlimited / 256MB default)
 * @throws \RuntimeException on corrupt data or if decompressed size exceeds $max_output_size
 */
function libdeflate_deflate_decompress(string $data, int $max_output_size = 0) : string;

/**
 * Equivalent to zlib_decode($data, $max_output_size)
 * @param int $max_output_size Maximum allowed output size in bytes (0 = unlimited / 256MB default)
 * @throws \RuntimeException on corrupt data or if decompressed size exceeds $max_output_size
 */
function libdeflate_zlib_decompress(string $data, int $max_output_size = 0) : string;

/**
 * Equivalent to gzdecode($data, $max_output_size)
 * @param int $max_output_size Maximum allowed output size in bytes (0 = unlimited / 256MB default)
 * @throws \RuntimeException on corrupt data or if decompressed size exceeds $max_output_size
 */
function libdeflate_gzip_decompress(string $data, int $max_output_size = 0) : string;
```
