/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.11 00:15:40
 *
\****************************************************/

#include <gtest/gtest.h>
#include <glog/logging.h>
#include "tiktoken.h"
#include "whisper_token.h"

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

TEST(Search, LLM) {
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
