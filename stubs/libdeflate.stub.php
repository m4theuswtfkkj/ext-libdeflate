<?php

/**
 * @generate-class-entries
 * @generate-legacy-arginfo 80100
 */

/**
 * Compresses data using raw DEFLATE format.
 *
 * @param string $data Raw uncompressed data.
 * @param int $level Compression level (1-12, default 6).
 * @return string Compressed data.
 * @throws \RuntimeException on compression failure.
 */
function libdeflate_deflate_compress(string $data, int $level = 6) : string {}

/**
 * Compresses data using ZLIB format (DEFLATE with ZLIB header and Adler-32 checksum).
 *
 * @param string $data Raw uncompressed data.
 * @param int $level Compression level (1-12, default 6).
 * @return string Compressed data.
 * @throws \RuntimeException on compression failure.
 */
function libdeflate_zlib_compress(string $data, int $level = 6) : string {}

/**
 * Compresses data using GZIP format (DEFLATE with GZIP header, CRC-32, and ISIZE footer).
 *
 * @param string $data Raw uncompressed data.
 * @param int $level Compression level (1-12, default 6).
 * @return string Compressed data.
 * @throws \RuntimeException on compression failure.
 */
function libdeflate_gzip_compress(string $data, int $level = 6) : string {}

/**
 * Decompresses raw DEFLATE data (equivalent to gzinflate).
 *
 * @param string $data Compressed data without headers.
 * @param int $max_output_size Maximum allowed output size in bytes (0 = default 256MB limit).
 * @return string Decompressed data.
 * @throws \RuntimeException on corrupt data or if output exceeds $max_output_size.
 */
function libdeflate_deflate_decompress(string $data, int $max_output_size = 0) : string {}

/**
 * Decompresses ZLIB data (equivalent to zlib_decode).
 *
 * @param string $data ZLIB-compressed data.
 * @param int $max_output_size Maximum allowed output size in bytes (0 = default 256MB limit).
 * @return string Decompressed data.
 * @throws \RuntimeException on corrupt data or if output exceeds $max_output_size.
 */
function libdeflate_zlib_decompress(string $data, int $max_output_size = 0) : string {}

/**
 * Decompresses GZIP data (equivalent to gzdecode).
 * Reads the uncompressed size directly from the footer for single-pass allocation.
 *
 * @param string $data GZIP-compressed data.
 * @param int $max_output_size Maximum allowed output size in bytes (0 = default 256MB limit).
 * @return string Decompressed data.
 * @throws \RuntimeException on corrupt data or if output exceeds $max_output_size.
 */
function libdeflate_gzip_decompress(string $data, int $max_output_size = 0) : string {}
