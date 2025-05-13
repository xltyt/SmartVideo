/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.13 10:50:23
 *
\****************************************************/

#include "frontend_utils.h"
#include <regex>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <string_utils.h>
#include "whisper_token.h"

bool contains_chinese(const std::string& text) {
  // 正则表达式：匹配一个或多个中文字符（基本汉字区 U+4E00–U+9FFF）
  PCRE2_SPTR pattern = (PCRE2_SPTR)"[\\x{4e00}-\\x{9fff}]+";
  int errornumber;
  PCRE2_SIZE erroroffset;

  // 编译正则表达式，启用 UTF 模式
  pcre2_code *re = pcre2_compile(
    pattern,
    PCRE2_ZERO_TERMINATED,
    PCRE2_UTF,
    &errornumber,
    &erroroffset,
    NULL
    );
  if (re == NULL) {
    return false;
  }

  // Match Data
  pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);
  if (match_data == NULL) {
    pcre2_code_free(re);
    return false;
  }

  //
  PCRE2_SPTR subject = (PCRE2_SPTR)text.c_str();
  int rc = pcre2_match(
    re,
    subject,
    PCRE2_ZERO_TERMINATED,
    0,
    0,
    match_data,
    NULL
    );

  // Free
  pcre2_match_data_free(match_data);
  pcre2_code_free(re);

  return rc >= 0;
}

std::string replace_blank(const std::string& text) {
  struct PosInfo {
    int offset;
    int len;
  };
  std::vector<PosInfo> pos_infos;
  {
    int len = text.size();
    int pos = 0;
    const char *p = text.c_str();
    for (pos = 0; pos < len; ) { 
      unsigned char c = *p; 
      int n = 0;
      if ((c & 0x80) == 0) {
        n = 1;
      }
      else if ((c & 0xE0) == 0xC0) {
        n = 2;
      }
      else if ((c & 0xF0) == 0xE0) {
        n = 3;
      }
      else if ((c & 0xF8) == 0xF0) {
        n = 4;
      }
      else if ((c & 0xFC) == 0xF8) {
        n = 5;
      }
      else if ((c & 0xFE) == 0xFC) {
        n = 6;
      }
      else {
        break;
      }
      if (pos + n > len) {
        break;
      }
      PosInfo info;
      info.offset = pos;
      info.len = n;
      pos_infos.push_back(info);
      p += n;
      pos += n;
    }
  }

  std::string result;
  for (int i = 0; i < pos_infos.size(); i++) {
    std::string token = std::string(text.data() + pos_infos[i].offset, pos_infos[i].len);
    if (token == " ") {
      if (0 == i || i == pos_infos.size() - 1) {
        continue;
      }
      std::string pre = std::string(text.data() + pos_infos[i - 1].offset, pos_infos[i - 1].len);
      std::string next = std::string(text.data() + pos_infos[i + 1].offset, pos_infos[i + 1].len);
      if (pre.size() == 1 && pre[0] <= 0x7F && pre != " " &&
          next.size() == 1 && next[0] <= 0x7F && next != " ") {
        result += token;
      }
    }
    else {
      result += token;
    }
  }

  return result;
}

void replace_corner_mark(std::string& text) {
  mycommon::str_replace(text, "²", "平方");
  mycommon::str_replace(text, "³", "立方");
}

void remove_bracket(std::string& text) {
  mycommon::str_replace(text, "（", "");
  mycommon::str_replace(text, "）", "");
  mycommon::str_replace(text, "【", "");
  mycommon::str_replace(text, "】", "");
  mycommon::str_replace(text, "`", "");
  mycommon::str_replace(text, "`", "");
  mycommon::str_replace(text, "——", " ");
}

std::string remove_tail_mark(const std::string& text) {
  // text = re.sub(r'[，,、]+$', '。', text)
	// 匹配字符串末尾一个或多个 ，, 、
  std::regex pattern(u8"[，,、]+$");
  return std::regex_replace(text, pattern, u8"。");
}

static std::string convertHundreds(int num, bool isLastChunk, bool hasPrevious, bool want_and) {
  static const char* ones[] = {
    "", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen",
    "seventeen", "eighteen", "nineteen"
  };
  static const char* tens[] = {
    "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
  };

  std::string result;
  int hundreds = num / 100;
  int remainder = num % 100;

  if (hundreds > 0) {
    result += std::string(ones[hundreds]) + " hundred";
    if (remainder > 0) {
      if (want_and) {
        result += " and ";
      }
      else {
        result += " ";
      }
    }
  }

  if (remainder > 0) {
    if (remainder < 20) {
      result += ones[remainder];
    }
    else {
      result += tens[remainder / 10];
      if (remainder % 10 > 0) {
        result += "-" + std::string(ones[remainder % 10]);
      }
    }
  }

  return result;
}

std::string number_to_words(long long num, bool want_and /*= true*/) {
  if (num == 0) {
    return "zero";
  }

  std::string result;
  bool negative = false;
  if (num < 0) {
    negative = true;
    num = -num;
  }

  // 将数字按千分位分组
  std::vector<int> chunks;
  while (num > 0) {
    chunks.push_back(static_cast<int>(num % 1000));
    num /= 1000;
  }
  std::reverse(chunks.begin(), chunks.end());

  static const char* scales[] = {"", "thousand", "million", "billion", "trillion",
    "quadrillion", "quintillion", "sextillion", "septillion",
    "octillion", "nonillion", "decillion"};

  int totalChunks = static_cast<int>(chunks.size());
  for (int i = 0; i < totalChunks; ++i) {
    int chunk = chunks[i];
    if (chunk == 0) continue;

    std::string chunkStr = convertHundreds(chunk, (i == totalChunks - 1), (i > 0 && chunks[i-1] != 0), want_and);
    if (!result.empty() && !chunkStr.empty()) {
      result += ", ";
    }
    result += chunkStr;

    int scaleIndex = totalChunks - 1 - i;
    if (scaleIndex > 0 && scaleIndex < 12) {
      result += " " + std::string(scales[scaleIndex]);
    }
  }

  if (negative) {
    result = "minus " + result;
  }

  return result;
}

std::string number_to_words(double num) {
  long long intPart = static_cast<long long>(num);
  double fracPart = num - intPart;
  if (fracPart < 0) fracPart = -fracPart;

  std::string result = number_to_words(intPart);
  if (fracPart > 0.0) {
    result += " point";
    // 处理小数部分，去掉前导零
    std::ostringstream oss;
    oss.precision(10);
    oss << std::fixed << fracPart;
    std::string fracStr = oss.str();
    // 跳过 "0."
    size_t dotPos = fracStr.find('.');
    if (dotPos != std::string::npos) {
      fracStr = fracStr.substr(dotPos + 1);
      // 去掉末尾多余的零
      while (!fracStr.empty() && fracStr.back() == '0') {
        fracStr.pop_back();
      }
    }
    for (char ch : fracStr) {
      result += " " + number_to_words(static_cast<long long>(ch - '0'), false);
    }
  }
  return result;
}

std::string spell_out_number(const std::string& text) {
  std::string new_text;
  int st = -1;
  for (int i = 0; i < text.size(); i++) {
    if (!isdigit(text[i])) {
      if (st >= 0) {
        std::string num_str = number_to_words((long long)atoi(text.substr(st, i - st).c_str()));
        new_text += num_str;
        st = -1;
      }
      new_text += text[i];
    }
    else {
      if (st < 0) {
        st = i;
      }
    }
  }
  if (st >= 0 && st < text.size()) {
    std::string num_str = number_to_words((long long)atoi(text.substr(st).c_str()));
    new_text += num_str;
  }
  return new_text;
}

std::vector<std::string> SplitUtf(const std::string& text) {
  std::vector<std::string> texts;
  int len = text.size();
  int pos = 0;
  const char *p = text.c_str();
  for (pos = 0; pos < len; ) { 
    unsigned char c = *p; 
    int n = 0;
    if ((c & 0x80) == 0) {
      n = 1;
    }
    else if ((c & 0xE0) == 0xC0) {
      n = 2;
    }
    else if ((c & 0xF0) == 0xE0) {
      n = 3;
    }
    else if ((c & 0xF8) == 0xF0) {
      n = 4;
    }
    else if ((c & 0xFC) == 0xF8) {
      n = 5;
    }
    else if ((c & 0xFE) == 0xFC) {
      n = 6;
    }
    else {
      break;
    }
    if (pos + n > len) {
      break;
    }
    texts.push_back(std::string(text.data() + pos, n));
    p += n;
    pos += n;
  }
  return texts;
}

std::vector<std::string> split_paragraph(
  const std::string& text_ori,
  WhisperToken *tokenize,
  const std::unordered_set<std::string>& allowed_special,
  const std::string lang /*= "zh"*/,
  int token_max_n /*= 80*/,
  int token_min_n /*= 60*/,
  int merge_len /*= 20*/,
  bool comma_split /*= false*/
  ) {
  std::string text = text_ori;
  std::set<std::string> pounc;
  if (lang == "zh") {
    pounc = std::set<std::string>({"。", "？", "！", "；", "：", "、", ".", "?", "!", ";"});
  }
  else {
    pounc = std::set<std::string>({".", "?", "!", ";", ":"});
  }
  if (comma_split) {
    pounc.insert("，");
    pounc.insert(",");
  }

  std::vector<std::string> words = SplitUtf(text);
  if (words.size() == 0) {
    return std::vector<std::string>();
  }
  if (pounc.count(words[words.size() - 1]) == 0) {
    if (lang == "zh") {
      text += "。";
      words.push_back("。");
    }
    else {
      text += ".";
      words.push_back(".");
    }
  }

  std::vector<std::string> utts;
  for (int i = 0, st = 0; i < (int)words.size(); i++) {
    const std::string& token = words[i];
    if (pounc.count(token) > 0) {
      std::string st_tmp;
      for (int k = st; k < i; k++) {
        st_tmp += words[k];
      }
      if (st_tmp.size()) {
        utts.push_back(st_tmp + token);
      }
      if (i + 1 < words.size() && std::set<std::string>({"\"", "”"}).count(words[i + 1]) > 0) {
        if (utts.size()) {
          std::string tmp = utts[utts.size() - 1];
          utts.pop_back();
          utts.push_back(tmp + words[i + 1]);
        }
        st = i + 2;
      }
      else {
        st = i + 1;
      }
    }
  }

  auto calc_utt_length = [&](const std::string& text) -> int {
    if (lang == "zh") {
      return mycommon::cacu_utf8_len(text);
    }
    else {
      return tokenize->encode(text, allowed_special).size();
    }
  };

  auto should_merge = [&](const std::string& text) -> bool {
    if (lang == "zh") {
      return mycommon::cacu_utf8_len(text) < merge_len;
    }
    else {
      return tokenize->encode(text, allowed_special).size() < merge_len;
    }
  };
  
  std::vector<std::string> final_utts;
  std::string cur_utt = "";
  for (auto utt : utts) {
    if (calc_utt_length(cur_utt + utt) > token_max_n && calc_utt_length(cur_utt) > token_min_n) {
      final_utts.push_back(cur_utt);
      cur_utt = "";
    }
    cur_utt = cur_utt + utt;
  }
  if (cur_utt.size() > 0) {
    if (should_merge(cur_utt) and final_utts.size() != 0) {
      final_utts[final_utts.size() - 1] += cur_utt;
    }
    else {
      final_utts.push_back(cur_utt);
    }
  }

  return final_utts;
}

bool is_only_punctuation(const std::string& text) {
  // Regular expression: Match strings that consist only of punctuation marks or are empty.
  // ^[\p{P}\p{S}]*$
  PCRE2_SPTR pattern = (PCRE2_SPTR)"^[\\p{P}\\p{S}]*$";

  int errorcode;
  PCRE2_SIZE erroroffset;

  //
  pcre2_code* re = pcre2_compile(
    pattern,
    PCRE2_ZERO_TERMINATED,
    PCRE2_UTF | PCRE2_UCP,
    &errorcode,
    &erroroffset,
    nullptr
    );

  if (!re) {
    PCRE2_UCHAR buffer;
    pcre2_get_error_message(errorcode, &buffer, sizeof(buffer));
    return false;
  }

  pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, nullptr);

  PCRE2_SPTR subject = (PCRE2_SPTR)text.c_str();
  int rc = pcre2_match(
    re,
    subject,
    text.size(),
    0,
    0,
    match_data,
    nullptr
    );

  pcre2_match_data_free(match_data);
  pcre2_code_free(re);

  return rc >= 0;
}

std::variant<std::string, std::vector<std::string>> text_normalize(
  WhisperToken *tokenize,
  const std::unordered_set<std::string>& allowed_special,
  const std::string& text_ori,
  bool split /*= true*/,
  bool text_frontend /*= true*/
  ) {
  std::string text = text_ori;
  if (!text_frontend || text.empty()) {
    if (split) {
      return std::vector<std::string>({text});
    }
    else {
      return text;
    }
  }
  std::vector<std::string> texts;
  mycommon::str_trim(text);
  if (contains_chinese(text)) {
    // TODO: Normalizer
    mycommon::str_replace(text, "\n", "");
    text = replace_blank(text);
    replace_corner_mark(text);
    mycommon::str_replace(text, ".", "。");
    mycommon::str_replace(text, " - ", "，");
    remove_bracket(text);
    text = remove_tail_mark(text);
    texts = split_paragraph(
      text,
      tokenize,
      allowed_special,
      "zh",
      80,
      60,
      20,
      false
      );
  }
  else {
    // TODO: Normalizer
    text = spell_out_number(text);
    texts = split_paragraph(
      text,
      tokenize,
      allowed_special,
      "en",
      80,
      60,
      20,
      false
      );
  }

  for (std::vector<std::string>::iterator iter = texts.begin(); iter != texts.end(); ) {
    if (is_only_punctuation(*iter)) {
      iter = texts.erase(iter);
    }
    else {
      iter++;
    }
  }
  if (split) {
    return texts;
  }
  else {
    return text;
  }
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
