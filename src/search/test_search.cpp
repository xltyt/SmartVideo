/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.13 10:51:46
 *
\****************************************************/

#include <gtest/gtest.h>
#include <glog/logging.h>
#include <crypt_utils.h>
#include <file_utils.h>
#include <string_utils.h>
#include "tiktoken.h"
#include "whisper_token.h"
#include "audio_utils.h"
#include "frontend_utils.h"

TEST(Search, Utf8) {
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġ");
    ASSERT_EQ(ids.size(), 1);
    ASSERT_EQ(ids[0], 288);
  }
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġt");
    ASSERT_EQ(ids.size(), 2);
    ASSERT_EQ(ids[0], 288);
    ASSERT_EQ(ids[1], 116);
  }
  {
    std::vector<uint32_t> ids = CoreBPE::decode_utf8("Ġbr");
    ASSERT_EQ(ids.size(), 3);
    ASSERT_EQ(ids[0], 288);
    ASSERT_EQ(ids[1], 98);
    ASSERT_EQ(ids[2], 114);
  }
}

TEST(Search, TikTokenEncodingGpt) {
  const std::string& ENDOFTEXT = "<|endoftext|>";
  const std::string& FIM_PREFIX = "<|fim_prefix|>";
  const std::string& FIM_MIDDLE = "<|fim_middle|>";
  const std::string& FIM_SUFFIX = "<|fim_suffix|>";
  const std::string& ENDOFPROMPT = "<|endofprompt|>";

  auto ToString = [](const std::vector<size_t>& ids) -> std::string {
    std::string str;
    for (int i = 0; i < ids.size(); i++) {
      if (str.size()) {
        str += " ";
      }
      str += std::to_string(ids[i]);
    }
    return str;
  };
  // gpt2
  CoreBPE::HashMap<std::vector<uint8_t>, size_t> mergeable_ranks = CoreBPE::data_gym_to_mergeable_bpe_ranks("data/gpt2/vocab.bpe", "data/gpt2/encoder.json");
  LOG(INFO) << "Convert BPE Finished, Len[" << mergeable_ranks.size() << "]";
  ASSERT_EQ(50256, mergeable_ranks.size());
  CoreBPE::StringMap<size_t> special_tokens;
  special_tokens[ENDOFTEXT] = 50256;
  TikTokenEncoding encoding(
    "gpt2",
    "'s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+",
    //"'s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+",
    //"'s|'t|'re|'ve|'m|'ll|'d|\\s+",
    mergeable_ranks,
    special_tokens,
    50257
    );
  LOG(INFO) << "Init Encoding Finished";
  ASSERT_EQ(true, encoding.encode("hello world") == std::vector<size_t>({31373, 995}));
  ASSERT_EQ(true, encoding.encode("hello <|endoftext|>", std::unordered_set<std::string>{"all"}) == std::vector<size_t>({31373, 220, 50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{"all"}) == std::vector<size_t>({50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{"<|endoftext|>"}) == std::vector<size_t>({50256}));
  ASSERT_EQ(true, encoding.encode("<|endoftext|>", std::unordered_set<std::string>{}, std::unordered_set<std::string>{}) == std::vector<size_t>({27, 91, 437, 1659, 5239, 91, 29}));
	try {
  	std::vector<size_t> ids = encoding.encode("<|endoftext|>");
    ASSERT_EQ(true, false);
	}
	catch (const std::exception& e) {
    ASSERT_EQ(true, true);
	}
	ASSERT_EQ(true, encoding.encode("0") == std::vector<size_t>({15}));
  ASSERT_EQ(true, encoding.encode("000000000") == std::vector<size_t>({10535, 830}));
  ASSERT_EQ(true, encoding.encode("00000000000000000") == std::vector<size_t>({8269, 10535, 830}));
  ASSERT_EQ(true, encoding.encode("今天天气真不错") == std::vector<size_t>({20015, 232, 25465, 25465, 36365, 242, 40367, 253, 38834, 165, 242, 247}));
  ASSERT_EQ(true, encoding.encode("i'm Jack") == std::vector<size_t>({72, 1101, 3619}));
  ASSERT_EQ(true, encoding.encode("") == std::vector<size_t>());
  LOG(INFO) << "Finished";
}

TEST(Search, TikTokenEncodingCl100K) {
  CoreBPE::HashMap<std::vector<uint8_t>, size_t> mergeable_ranks = CoreBPE::load_tiktoken_bpe("data/cl100k_base/cl100k_base.tiktoken");
  LOG(INFO) << "Convert BPE Finished, Len[" << mergeable_ranks.size() << "]";
  ASSERT_EQ(100256, mergeable_ranks.size());
  CoreBPE::StringMap<size_t> special_tokens;
  special_tokens["ENDOFTEXT"] = 100257;
  special_tokens["FIM_PREFIX"] = 100258;
  special_tokens["FIM_MIDDLE"] = 100259;
  special_tokens["FIM_SUFFIX"] = 100260;
  special_tokens["ENDOFPROMPT"] = 100276;
  TikTokenEncoding encoding(
    "cl100k_base",
    "(?i:'s|'t|'re|'ve|'m|'ll|'d)|[^\\r\\n\\p{L}\\p{N}]?\\p{L}+|\\p{N}{1,3}| ?[^\\s\\p{L}\\p{N}]+[\\r\\n]*|\\s*[\\r\\n]+|\\s+(?!\\S)|\\s+",
    mergeable_ranks,
    special_tokens,
    50257
    );
  LOG(INFO) << "Init Encoding Finished";
  ASSERT_EQ(true, encoding.encode("rer") == std::vector<size_t>({38149}));
  ASSERT_EQ(true, encoding.encode("'rer") == std::vector<size_t>({2351, 81}));
  ASSERT_EQ(true, encoding.encode("today\n ") == std::vector<size_t>({31213, 198, 220}));
  ASSERT_EQ(true, encoding.encode("today\n \n") == std::vector<size_t>({31213, 27907}));
  ASSERT_EQ(true, encoding.encode("today\n  \n") == std::vector<size_t>({31213, 14211}));
  ASSERT_EQ(true, encoding.encode("hello world") == std::vector<size_t>({15339, 1917}));
  //std::vector<size_t> ids = encoding.encode(" \x85""0");
  //LOG(INFO) << ids.size();
  //for (auto id : ids) {
  //  LOG(INFO) << id;
  //}
  //ASSERT_EQ(true, encoding.encode(" \x85""0") == std::vector<size_t>({220, 126, 227, 15}));
    
  ASSERT_EQ(true, encoding.encode("👍") == std::vector<size_t>({9468, 239, 235}));
  //
  //  # surrogate pair gets converted to codepoint
  //  assert enc.encode("") == []
  //  # lone surrogate just gets replaced
  //  assert enc.encode("\ud83d") == enc.encode("�")
  //ASSERT_EQ(true, encoding.encode("\ud83d\udc4d") == std::vector<size_t>({9468, 239, 235}));
  //ASSERT_EQ(true, encoding.encode("") == std::vector<size_t>({}));
  LOG(INFO) << "Finished";
}

TEST(Search, TikTokenEncodingR50K) {
}

TEST(Search, WhisperToken) {
  const std::string text = "다람쥐 헌 쳇바퀴에 타고파";
  WhisperToken token(".");
  std::vector<size_t> multilingual_tokens = token.encode(text);
  ASSERT_EQ(true, multilingual_tokens == std::vector<size_t>({9835, 22855, 168, 98, 238, 13431, 234, 43517, 229, 47053, 169, 222, 19086, 19840, 1313, 17974}));
}

TEST(Search, Wav) {
  const std::string& path = "data/mda-qmwfy2k746929rxh.mp3";
  std::string output_path = "out_" + Crypt::gen_random_string(8) + ".wav";
  ConvertToWav(path, output_path);
  std::vector<float> result;
  int ret = LoadWav(output_path, 160000, result);
  ASSERT_EQ(ret, -1);
  ret = LoadWav(output_path, 16000, result);
  ASSERT_EQ(ret, 0);
  LOG(INFO) << "Len[" << result.size() << "]";
  unlink(output_path.c_str());
  mycommon::file_write("data.txt", mycommon::str_join(result, "\n"));
}

TEST(Search, FrontendUtils) {
  ASSERT_EQ(true, contains_chinese("中国"));
  ASSERT_EQ(true, contains_chinese("1中2国3"));
  ASSERT_EQ(true, contains_chinese("12国3"));
  ASSERT_EQ(false, contains_chinese("123"));
  ASSERT_EQ(false, contains_chinese("a"));

  ASSERT_EQ(true, replace_blank("a b") == "a b");
  ASSERT_EQ(true, replace_blank("a  b") == "ab");
  ASSERT_EQ(true, replace_blank("中 国") == "中国");
  ASSERT_EQ(true, replace_blank("中  国") == "中国");
  ASSERT_EQ(true, replace_blank("中 a") == "中a");
  ASSERT_EQ(true, replace_blank("中  a") == "中a");
  
  ASSERT_EQ(true, remove_tail_mark("中国，") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，,") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国，，,、、") == "中国。");
  ASSERT_EQ(true, remove_tail_mark("中国A，，,、、") == "中国A。");
  
  ASSERT_EQ(false, is_only_punctuation("中国"));
  ASSERT_EQ(true, is_only_punctuation("."));
  ASSERT_EQ(true, is_only_punctuation("。"));
  ASSERT_EQ(true, is_only_punctuation("。、."));

  ASSERT_EQ(true, number_to_words((long long)0) == "zero");
  ASSERT_EQ(true, number_to_words((long long)1) == "one");
  ASSERT_EQ(true, number_to_words((long long)100) == "one hundred");
  ASSERT_EQ(true, number_to_words((long long)123) == "one hundred and twenty-three");
  ASSERT_EQ(true, number_to_words((long long)12345) == "twelve thousand, three hundred and forty-five");
  ASSERT_EQ(true, number_to_words((long long)-1) == "minus one");
  
  ASSERT_EQ(true, number_to_words(123.2) == "one hundred and twenty-three point two");
  ASSERT_EQ(true, number_to_words(123.23456) == "one hundred and twenty-three point two three four five six");
  ASSERT_EQ(true, number_to_words(-123.2) == "minus one hundred and twenty-three point two");
  
  ASSERT_EQ(true, spell_out_number("A 123 B") == "A one hundred and twenty-three B");

  WhisperToken tokenize(".");
  {
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), "中国A，，,、、", false, true);
    if (auto* str = std::get_if<std::string>(&result)) {
      const std::string& word = *str;
      LOG(INFO) << "Match[" << word << "]";
      ASSERT_EQ(true, word == "中国A。");
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    std::string text = "家事国事天下事，让人民过上幸福生活是头等大事。家家户户都盼着孩子能有好的教育，老人能有好的养老服务，年轻人能有更多发展机会。这些朴实的愿望，";
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), text, true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    std::string text = "习近平总书记高度重视新型基础设施发展，不仅对构建新型基础设施体系作出全局谋划，提出“要适度超前，布局有利于引领产业发展和维护国家安全的基础设施”，“打造集约高效、经济适用、智能绿色、安全可靠的现代化基础设施体系”，并对网络强国、数字中国、交通强国、能源强国等作出一系列具体部署，强调要“加快建设高速泛在、天地一体、云网融合、智能敏捷、绿色低碳、安全可控的智能化综合性数字信息基础设施”，为新时代推动新型基础设施发展提供了思想指引和根本遵循。";
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), text, true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
  {
    auto result = text_normalize(&tokenize, std::unordered_set<std::string>({"all"}), "中国A，，,、、", true, true);
    if (auto* vec = std::get_if<std::vector<std::string>>(&result)) {
      for (auto word : *vec) {
        LOG(INFO) << "Match[" << word << "]";
      }
    }
    else {
      ASSERT_EQ(false, true);
    }
  }
}

TEST(Search, Text) {
}

TEST(Search, LLM) {
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
