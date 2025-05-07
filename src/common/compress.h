/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.07 17:44:21
 *
\****************************************************/

#ifndef _COMPRESS_H__
#define _COMPRESS_H__

#include <functional>
#include <memory>

namespace common {
  enum COMPRESSION_TYPE : unsigned char {
    NO_COMPRESSION = 0X0,
    SNAPPY_COMPRESSION,
    LZ4_COMPRESSION,
    ZSTD_COMPRESSION,
    MAX_COMPRESSION
  };
#if 0
  bool Compress(const std::string& str, std::string& dst, COMPRESSION_TYPE type = LZ4_COMPRESSION);
  bool DeCompress(const std::string& str, std::string& dst, COMPRESSION_TYPE type = LZ4_COMPRESSION);
  
  bool Compress_lz4(const std::string& str, std::string& dst);
  bool DeCompress_lz4(const std::string& str, std::string& dst);
  bool Compress_snappy(const std::string& str, std::string& dst);
  bool DeCompress_snappy(const std::string& str, std::string& dst);
#endif
}

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
