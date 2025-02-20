#include "bmf_reader/bmf_reader.h"

#include <cassert>
#include <optional>

constexpr size_t BMF_HEADER_BYTE_SIZE = 4;
constexpr size_t BMF_BLOCK_PREFIX_BYTE_SIZE = 5;

constexpr int8_t BMF_INFO_BLOCK_ID = 1;
constexpr int8_t BMF_COMMON_BLOCK_ID = 2;
constexpr int8_t BMF_PAGES_BLOCK_ID = 3;
constexpr int8_t BMF_CHARS_BLOCK_ID = 4;

#pragma pack(push, 1)
struct BmfInfoBlock {};
#pragma pack(pop)

#pragma pack(push, 1)
struct BmfCommonBlock {};
#pragma pack(pop)

using block_type_id_t = int8_t;
using block_size_in_bytes_t = int32_t;

namespace {
  enum class EnumCallbackResult { Continue, Stop };

  template<typename Func>
  BmfReadResult enumerate_blocks(
      const std::span<const unsigned char> remaining_file_bytes,
      Func&& block_handler_callback) {
    auto unread_bytes = remaining_file_bytes;

    while (unread_bytes.size_bytes() >= BMF_BLOCK_PREFIX_BYTE_SIZE) {
      // Read block type and size prior to reading the contents of the block.
      const block_type_id_t block_type_id = unread_bytes[0];
      unread_bytes = unread_bytes.subspan(sizeof(block_type_id_t));

      // Verify the block id matches a known type.
      if (block_type_id != BMF_INFO_BLOCK_ID &&
          block_type_id != BMF_COMMON_BLOCK_ID &&
          block_type_id != BMF_PAGES_BLOCK_ID &&
          block_type_id != BMF_CHARS_BLOCK_ID) {
        return BmfReadResult::InvalidBlockType;
      }

      block_size_in_bytes_t block_size_in_bytes = 0;
      std::memcpy(
          &block_size_in_bytes,
          unread_bytes.data(),
          sizeof(block_size_in_bytes));
      unread_bytes = unread_bytes.subspan(sizeof(block_size_in_bytes));

      // Verify the block size is not of bounds.
      if (block_size_in_bytes <= 0 ||
          block_size_in_bytes >= unread_bytes.size_bytes()) {
        return BmfReadResult::InvalidBlockSize;
      }

      // Read the block data and use the callback function to let the caller
      // handle whatever it contains.
      // TODO: Bail if callback returns false
      const auto block_bytes = unread_bytes.subspan(0, block_size_in_bytes);

      if (block_handler_callback(
              block_type_id, block_size_in_bytes, block_bytes) ==
          EnumCallbackResult::Stop) {
        break;
      }

      // Advance the buffer span to the start of the next unread block header.
      unread_bytes = unread_bytes.subspan(block_size_in_bytes);
    }

    // Done!
    return BmfReadResult::Ok;
  }
} // namespace

BmfReadResult read_bmfont(const std::span<const unsigned char> file_bytes) {
  // First three bytes must be "BMF" (66, 77, 70).
  if (file_bytes.size_bytes() < BMF_HEADER_BYTE_SIZE || file_bytes[0] != 66 ||
      file_bytes[1] != 77 || file_bytes[2] != 70) {
    return BmfReadResult::NotABmfFile;
  }

  // The fourth byte is the version. Only version 3 is supported by this reader.
  if (file_bytes[3] != 3) {
    return BmfReadResult::VersionNotSupported;
  }

  // Move the buffer span forward past the header bytes that were just read.
  const auto bytes_after_header = file_bytes.subspan(BMF_HEADER_BYTE_SIZE);

  // Locate the BMF info and common block headers prior to reading the other
  // blocks.
  std::optional<BmfInfoBlock> info_block = {};
  std::optional<BmfCommonBlock> common_block = {};

  auto read_result = enumerate_blocks(
      bytes_after_header,
      [&info_block, &common_block](
          block_type_id_t block_type_id,
          block_size_in_bytes_t block_size_in_bytes,
          const std::span<const unsigned char> block_bytes) {
        switch (block_type_id) {
          case BMF_INFO_BLOCK_ID: {
            BmfInfoBlock raw_info_block;
            assert(block_size_in_bytes == sizeof(BmfInfoBlock));

            std::memcpy(
                &raw_info_block, block_bytes.data(), block_size_in_bytes);

            info_block = raw_info_block;
          } break;
          case BMF_COMMON_BLOCK_ID:
            break;
          default:
            break;
        }

        // Stop enumerating blocks once the info and common blocks have been
        // located.
        return info_block.has_value() && common_block.has_value()
                   ? EnumCallbackResult::Stop
                   : EnumCallbackResult::Continue;
      });

  if (read_result != BmfReadResult::Ok) {
    return read_result;
  }

  return BmfReadResult::Ok;
}