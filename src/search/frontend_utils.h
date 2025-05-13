/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.13 10:49:25
 *
\****************************************************/

#ifndef _FRONTEND_UTILS_H__
#define _FRONTEND_UTILS_H__

#include <string>
#include <vector>
#include <variant>
#include <unordered_set>

// whether contain chinese character
bool contains_chinese(const std::string& text);

// remove blank between chinese character
std::string replace_blank(const std::string& text);

// replace special symbol
void replace_corner_mark(std::string& text);

// remove meaningless symbol
void remove_bracket(std::string& text);

// remove tail mark
std::string remove_tail_mark(const std::string& text);

std::string number_to_words(long long num, bool want_and = true);
std::string number_to_words(double num);

// spell Arabic numerals
std::string spell_out_number(const std::string& text);

std::vector<std::string> SplitUtf(const std::string& text);

// split paragrah logic：
// 1. per sentence max len token_max_n, min len token_min_n, merge if last sentence len less than merge_len
// 2. cal sentence len according to lang
// 3. split sentence according to puncatation
class WhisperToken;
std::vector<std::string> split_paragraph(
  const std::string& text,
  WhisperToken *tokenize,
  const std::unordered_set<std::string>& allowed_special,
  const std::string lang = "zh",
  int token_max_n = 80,
  int token_min_n = 60,
  int merge_len = 20,
  bool comma_split = false
  );

bool is_only_punctuation(const std::string& text);

std::variant<std::string, std::vector<std::string>> text_normalize(
  WhisperToken *tokenize,
  const std::unordered_set<std::string>& allowed_special,
  const std::string& text,
  bool split = true,
  bool text_frontend = true
  );

#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
