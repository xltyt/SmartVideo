/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.07 17:44:22
 *
\****************************************************/

#include "compress.h"

#if 0
#include <lz4.h>
#include <snappy-c.h>
#endif

#if 0
namespace common {
  bool Compress(const std::string& str, std::string& dst, COMPRESSION_TYPE type /*= LZ4_COMPRESSION*/) {
    if (LZ4_COMPRESSION == type) {
      return Compress_lz4(str, dst);
    }
    if (SNAPPY_COMPRESSION == type) {
      return Compress_snappy(str, dst);
    }
    return false;
  }

  bool DeCompress(const std::string& str, std::string& dst, COMPRESSION_TYPE type /*= LZ4_COMPRESSION*/) {
    if (LZ4_COMPRESSION == type) {
      return DeCompress_lz4(str, dst);
    }
    if (SNAPPY_COMPRESSION == type) {
      return DeCompress_snappy(str, dst);
    }
    return false;
  }
  
  bool Compress_lz4(const std::string& str, std::string& dst) {
    const int src_size = (int)(str.size());
    const int max_dst_size = LZ4_compressBound(src_size);
    char *compressed_data = (char *)malloc(max_dst_size + sizeof(uint32_t));
    if (NULL == compressed_data) {
      return false;
    }
    *((uint32_t *)compressed_data) = src_size;
    const int compressed_data_size = LZ4_compress_default(str.c_str(), compressed_data + sizeof(uint32_t), src_size, max_dst_size);
    if (compressed_data_size <= 0) {
      free(compressed_data);
      return false;
    }
    //fprintf(stderr, "%d\n", compressed_data_size + sizeof(uint32_t));
    dst = std::string(compressed_data, compressed_data_size + sizeof(uint32_t));
    //fprintf(stderr, "%d\n", dst.size());
    free(compressed_data);
    return true;
  }

  bool DeCompress_lz4(const std::string& str, std::string& dst) {
    //fprintf(stderr, "%d\n", str.size());
    size_t src_size = *((uint32_t *)str.c_str());
    //fprintf(stderr, "%d\n", src_size);
    char *decompressed_data = (char *)malloc(src_size);
    if (NULL == decompressed_data) {
      return false;
    }
    const int decompressed_size = LZ4_decompress_safe(str.c_str() + sizeof(uint32_t), decompressed_data, str.size() - sizeof(uint32_t), src_size);
    //fprintf(stderr, "%d\n", decompressed_size);
    if (decompressed_size <= 0) {
      free(decompressed_data);
      return false;
    }
    dst = std::string(decompressed_data, decompressed_size);
    free(decompressed_data);
    //fprintf(stderr, "%d\n", dst.size());
    return true;
  }
  
  bool Compress_snappy(const std::string& str, std::string& dst) {
    const int src_size = (int)(str.size());
    size_t output_length = snappy_max_compressed_length(src_size);
    char* output = (char*)malloc(output_length);
    if (NULL == output) {
      return false;
    }
    bool ret = false;
    if (snappy_compress(str.c_str(), src_size, output, &output_length) == SNAPPY_OK) {
      dst = std::string(output, output_length);
      ret = true;
    }
    free(output);
    return ret;
  }

  bool DeCompress_snappy(const std::string& str, std::string& dst) {   
    size_t output_length = 0;
    if (snappy_uncompressed_length(str.c_str(), str.size(), &output_length) != SNAPPY_OK) {
      return false;
    }
    char* output = (char*)malloc(output_length);
    if (NULL == output) {
      return false;
    }
    bool ret = false;
    if (snappy_uncompress(str.c_str(), str.size(), output, &output_length) == SNAPPY_OK) {
      dst = std::string(output, output_length);
      ret = true;
    }
    free(output);
    return ret;
  }
}
#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
