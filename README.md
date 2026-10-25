# ext-libdeflate
[![CI](https://github.com/pmmp/ext-libdeflate/actions/workflows/ci.yml/badge.svg)](https://github.com/pmmp/ext-libdeflate/actions/workflows/ci.yml)

PHP bindings for [libdeflate](https://github.com/ebiggers/libdeflate), a zlib replacement with significantly better performance.

## Features
This extension exposes libdeflate's whole-buffer compression and decompression APIs for DEFLATE, ZLIB, and GZIP formats.

Decompression is implemented with dynamic exponential buffer scaling and optional maximum output size protection to prevent decompression bomb vulnerabilities. In addition, GZIP decompression automatically reads the uncompressed size from the GZIP footer when available for optimal single-pass decompression.

## Performance
Compared to `zlib_encode()`, a 40% performance improvement can be observed at level 1, rising to 50-70% at level 6 (default), for approximately the same compression ratios. Also, `libdeflate_deflate_compress()` was observed to sometimes be faster at level 6 compression than `zlib_encode()` at level 1 compression.

Decompression is typically 2x to 3x faster than standard `zlib_decode()` / `gzinflate()` / `gzdecode()`.

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
