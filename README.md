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

### Decompression Benchmarks (Measured On-Device)

#### Device Specifications
- **Device Model:** Samsung Galaxy A05 (`SM-A055M`)
- **Processor / SoC:** MediaTek Helio G85 (`MT6769V/CZ`), 8 cores (2x 2.0 GHz Cortex-A75 & 6x 1.8 GHz Cortex-A55)
- **Architecture:** ARMv8.2-A / aarch64 (features: fp, asimd, aes, pmull, crc32, atomics, asimddp)
- **RAM:** 4 GB LPDDR4x (3.75 GB usable)
- **Operating System:** Android 15 (Kernel `6.6.89-android15-8-abA055MUBSJDZF1-4k`, Termux environment)
- **PHP Environment:** PHP 8.5.12 (CLI, ZTS, 64-bit musl, `pmmpthread` v6.4.0)

#### Decompression Results
Measured against PHP's built-in `zlib_decode()` across 2,000 operations per payload:

| Payload Format & Size | `zlib_decode()` | `libdeflate_*_decompress()` | Speedup |
|:----------------------|:----------------|:----------------------------|:--------|
| **ZLIB (1 KB)**       | 43.74 ms        | 9.31 ms                     | **4.70x faster** |
| **GZIP (1 KB)**       | 41.64 ms        | 5.74 ms                     | **7.25x faster** |
| **ZLIB (16 KB)**      | 208.01 ms       | 86.61 ms                    | **2.40x faster** |
| **GZIP (16 KB)**      | 235.07 ms       | 43.10 ms                    | **5.45x faster** |
| **ZLIB (64 KB)**      | 554.91 ms       | 250.08 ms                   | **2.22x faster** |
| **GZIP (64 KB)**      | 731.33 ms       | 125.63 ms                   | **5.82x faster** |

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
