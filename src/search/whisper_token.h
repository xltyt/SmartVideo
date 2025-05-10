/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.11 00:02:54
 *
\****************************************************/

#ifndef _WHISPER_TOKEN_H__
#define _WHISPER_TOKEN_H__

#include "tiktoken.h"

class WhisperToken {
public:
  WhisperToken(const std::string& dir);
  virtual ~WhisperToken();

public:
  std::vector<size_t> encode(
    const std::string& text,
    const std::unordered_set<std::string>& allowed_special = std::unordered_set<std::string>(),
    const std::unordered_set<std::string>& disallowed_special = std::unordered_set<std::string>({"all"})
    );

protected:
  TikTokenEncoding *_encoding = NULL;
};

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
