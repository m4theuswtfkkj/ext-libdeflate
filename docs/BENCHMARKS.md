# ext-libdeflate benchmarks

All numbers were measured on-device (Termux).

## Environment
- **Device:** Samsung Galaxy A05 (`SM-A055M`), MediaTek Helio G85 (2x Cortex-A75 + 6x Cortex-A55), 4 GB RAM
- **Architecture:** ARMv8.2-A / aarch64 (fp, asimd, aes, pmull, crc32, atomics, asimddp)
- **OS:** Android 15, Termux
- **PHP:** 8.5.12 (CLI, ZTS, 64-bit musl, `pmmpthread` v6.4.0)

## Results

Measured against PHP's `zlib_decode()`, 2,000 operations per payload. (Taken before the compress-path allocation change below; not re-measured because `libdeflate` headers are not installed on the device.)

| Payload | `zlib_decode()` | `libdeflate_*_decompress()` | Speedup |
|:--|--:|--:|:--|
| ZLIB 1 KB | 43.74 ms | 9.31 ms | 4.70x |
| GZIP 1 KB | 41.64 ms | 5.74 ms | 7.25x |
| ZLIB 16 KB | 208.01 ms | 86.61 ms | 2.40x |
| GZIP 16 KB | 235.07 ms | 43.10 ms | 5.45x |
| ZLIB 64 KB | 554.91 ms | 250.08 ms | 2.22x |
| GZIP 64 KB | 731.33 ms | 125.63 ms | 5.82x |

Compression: ~40% faster than `zlib_encode()` at level 1, 50–70% at level 6.


## Remaining optimization candidates
- libdeflate: ZLIB/DEFLATE decompression guesses `4×` input and doubles on failure, which can re-run decompression several times; storing a size hint is not possible in the format, so callers that know the size should pass `$max_output_size`.
