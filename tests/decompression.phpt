--TEST--
Test that decompression works normally and matches original data
--SKIPIF--
<?php if(!extension_loaded("libdeflate")) die("skip: extension not loaded"); ?>
--FILE--
<?php

$original = str_repeat("The quick brown fox jumps over the lazy dog. 1234567890!@#$%", 200);

$deflateCompressed = libdeflate_deflate_compress($original, 6);
$zlibCompressed = libdeflate_zlib_compress($original, 6);
$gzipCompressed = libdeflate_gzip_compress($original, 6);

// Test raw deflate
$unDeflate = libdeflate_deflate_decompress($deflateCompressed);
var_dump($unDeflate === $original);

// Test zlib
$unZlib = libdeflate_zlib_decompress($zlibCompressed);
var_dump($unZlib === $original);

// Test gzip
$unGzip = libdeflate_gzip_decompress($gzipCompressed);
var_dump($unGzip === $original);

// Test with max_output_size limit
$unZlibLimit = libdeflate_zlib_decompress($zlibCompressed, strlen($original));
var_dump($unZlibLimit === $original);

// Test with too small limit (should throw RuntimeException)
try {
    libdeflate_zlib_decompress($zlibCompressed, 50);
    echo "FAIL: expected exception on small limit\n";
} catch (\RuntimeException $e) {
    echo "OK: limit exceeded exception caught\n";
}

// Test corrupt data
try {
    libdeflate_zlib_decompress("invalid corrupt data string");
    echo "FAIL: expected exception on corrupt data\n";
} catch (\RuntimeException $e) {
    echo "OK: corrupt data exception caught\n";
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
OK: limit exceeded exception caught
OK: corrupt data exception caught
