#pragma once

#include <span>

enum class BmfReadResult {
  Ok,                  /// The BMF file was read succesfully.
  NotABmfFile,         /// The file lacks the expected BMF file header.
  VersionNotSupported, /// The version in the BMF file is not supported.
  InvalidBlockType,    /// The block type id was not recogonized.
  InvalidBlockSize,    /// Block size is larger than the remainder of the file.
};

BmfReadResult read_bmfont(std::span<const unsigned char> file_bytes);