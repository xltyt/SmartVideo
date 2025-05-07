#include <gtest/gtest.h>
#include <math.h>
#include <glog/logging.h>
#include <net_utils.h>
#include <timer.h>
#include <crypt_utils.h>
#include <sqlite_utils.h>
#include <string_utils.h>
#include <common.h>

TEST(Common, Time) {
  uint32_t t = mycommon::parseTime("2025-01-01 02:02:02");
  ASSERT_NE(t, (uint32_t)-1);
  t = mycommon::parseTime("2025-xx-01 02:02:02");
  ASSERT_EQ(t, (uint32_t)-1);
  t = mycommon::parseTime("2025-13-01 02:02:02");
  ASSERT_EQ(t, (uint32_t)-1);
  t = mycommon::parseTime("2025-02-31 02:02:02");
  ASSERT_EQ(t, (uint32_t)-1);
}

TEST(Common, Es1) {
  //std::vector<std::string> media_descs;
  //std::vector<std::string> media_full;
  //FetchEsReault(_timeout_connect, _timeout_read, location_config.es_url, location_config.es_cf_index, "西安 民警 暴雨夜", "", 100, true, media_descs, media_full, "comment_count", true);
  //for (int i = 0; i < media_full.size(); i++) {
  //  LOG(INFO) << media_full[i];
  //}
}

TEST(Common, SqliteUtils) {
	mycommon::SqliteUtils sqlite("test.db");
  int ret = sqlite.connect();
  ASSERT_EQ(0, ret);
  LOG(INFO) << sqlite3_version;
  int workset_id = 267;
  int group_id = 100;
  char name[128];
  sprintf(name, "yuqing_group_stats_%d_%d", workset_id, group_id);
  {
    std::vector<std::string> sqls;
    char sql[4096] = {0};
    sprintf(sql, "CREATE TABLE IF NOT EXISTS %s ( \
      id INTEGER PRIMARY KEY AUTOINCREMENT, \
      time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, \
      unique_id TEXT UNIQUE, \
      post_create_time TIMESTAMP DEFAULT '2000-01-01 00:00:00', \
      post_create_time_min TEXT, \
      post_create_time_hour TEXT, \
      post_create_time_day TEXT, \
      comment_count INTEGER, \
      repost_count INTEGER, \
      like_count INTEGER, \
      platform TEXT, \
      account_type TEXT, \
      media_type TEXT, \
      emotion_type TEXT, \
      nickname TEXT \
      );", name);
    sqls.push_back(sql);
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_time ON %s(time); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_post_create_time ON %s(post_create_time); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_post_create_time_min ON %s(post_create_time_min); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_post_create_time_hour ON %s(post_create_time_hour); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_post_create_time_day ON %s(post_create_time_day); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_platform ON %s(platform); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_account_type ON %s(account_type); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_media_type ON %s(media_type); ", name));
    sqls.push_back(mycommon::str_format("CREATE INDEX idx_emotion_type ON %s(emotion_type); ", name));
    ret = sqlite.execute(sqls);
    ASSERT_EQ(0, ret);
  }
  {
    ret = sqlite.execute(mycommon::str_format("INSERT INTO %s(`unique_id`,`post_create_time`,`post_create_time_min`,`post_create_time_hour`,`post_create_time_day`,`comment_count`,`repost_count`,`like_count`,`platform`,`account_type`,`media_type`,`emotion_type`,`nickname`) VALUES('1','2025-01-01 11:00:00','2025-01-01 11:00','2025-01-01 11','2025-01-01',100,200,300,'douyin','media','123','negative','n1')", name));
    ASSERT_EQ(1, ret);
    ASSERT_EQ(1, sqlite.last_id());
    mycommon::SqliteUtils::ResultSetMap result;
    ret = sqlite.fetch_format(result, "SELECT * FROM %s", name);
    ASSERT_EQ(ret, 1);
    ASSERT_EQ(result.data().size(), 1);
    for (std::vector<std::unordered_map<std::string, std::string> >::const_iterator iter = result.data().begin(); iter != result.data().end(); iter++) {
      const std::unordered_map<std::string, std::string>& fields = *iter;
      std::string unique_id = result.get_field(fields, "unique_id");
      std::string post_create_time = result.get_field(fields, "post_create_time");
      LOG(INFO) << "UniqueId[" << unique_id << "] Time[" << post_create_time << "]";
    }
    ret = sqlite.execute(mycommon::str_format("DELETE FROM %s", name));
    ASSERT_EQ(1, ret);
    ret = sqlite.execute(mycommon::str_format("UPDATE sqlite_sequence SET seq=0 WHERE name='%s'", name));
    ASSERT_EQ(1, ret);
  }
  for (int i = 0; i < 1000; i++) {
    ret = sqlite.execute(mycommon::str_format("INSERT INTO %s(`unique_id`,`post_create_time`,`post_create_time_min`,`post_create_time_hour`,`post_create_time_day`,`comment_count`,`repost_count`,`like_count`,`platform`,`account_type`,`media_type`,`emotion_type`,`nickname`) VALUES('%d','2025-01-01 11:00:00','2025-01-01 11:00','2025-01-01 11','2025-01-01',%d,%d,%d,'douyin','media','123','negative','n1')", name, i, 100 + i, 200 + i, 300 + i));
    ASSERT_EQ(1, ret);
    ASSERT_EQ(i + 1, sqlite.last_id());
  }
  int ct = 0;
  ret = sqlite.fetch_format_scalar(-1, ct, "SELECT COUNT(*) FROM %s", name);
  ASSERT_EQ(0, ret);
  ASSERT_EQ(1000, ct);
}

#if 0
TEST(Common, Lock) {
  EtcdUtils *etcd[10];
  for (int i = 0; i < 10; i++) {
    etcd[i] = new EtcdUtils("rw-1640bc.etcd.qihudb.net", 2078, "etcd", "7dbdf6674b90f01d");
    int ret = etcd[i]->Init();
    ASSERT_EQ(0, ret);
  }
  int result = 0;
  auto job_results = std::vector<std::future<void>>();
  common::RWLock caculate_lock;
  for (int i = 0; i < 10; i++) {
    job_results.push_back(std::async(std::launch::async, [i, &etcd, &result, &caculate_lock]() -> void {
      LOG(INFO) << "[" << i << "] Lock";
      std::string result_path;
      int ret = etcd[i]->Lock("/test_lock", result_path);
      result++;
      LOG(INFO) << "[" << i << "] Unlock";
      ret = etcd[i]->Unlock(result_path);
      LOG(INFO) << "[" << i << "] OK";
    }));
  }
  for (auto &result : job_results) {
    result.get();
  }
  for (int i = 0; i < 10; i++) {
    delete etcd[i];
  }
  ASSERT_EQ(result, 10);
}

TEST(Common, Lock1) {
  int result = 0;
  auto job_results = std::vector<std::future<void>>();
  common::RWLock caculate_lock;
  for (int i = 0; i < 10; i++) {
    job_results.push_back(std::async(std::launch::async, [i, &result, &caculate_lock]() -> void {
      EtcdUtils *etcd = new EtcdUtils("rw-1640bc.etcd.qihudb.net", 2078, "etcd", "7dbdf6674b90f01d");
      int ret = etcd->Init();
      ASSERT_EQ(ret, 0);
      LOG(INFO) << "[" << i << "] Lock";
      std::string result_path;
      ret = etcd->Lock("/test_lock", result_path);
      ASSERT_EQ(ret, 0);
      result++;
      LOG(INFO) << "[" << i << "] Unlock";
      ret = etcd->Unlock(result_path);
      LOG(INFO) << "[" << i << "] OK";
      delete etcd;
    }));
  }
  for (auto &result : job_results) {
    result.get();
  }
  ASSERT_EQ(result, 10);
}

TEST(Common, Lock2) {
  EtcdUtils *etcd = new EtcdUtils("rw-1640bc.etcd.qihudb.net", 2078, "etcd", "7dbdf6674b90f01d");
  int ret = etcd->Init();
  ASSERT_EQ(ret, 0);
  {
    int result = 0;
    auto job_results = std::vector<std::future<void>>();
    for (int i = 0; i < 10; i++) {
      job_results.push_back(std::async(std::launch::async, [&etcd, i, &result]() -> void {
        LOG(INFO) << "[" << i << "] Lock";
        std::string result_path;
        int ret = etcd->Lock("/test_lock", result_path);
        ASSERT_EQ(ret, 0);
        result++;
        LOG(INFO) << "[" << i << "] Unlock";
        ret = etcd->Unlock(result_path);
        ASSERT_EQ(ret, 0);
        LOG(INFO) << "[" << i << "] OK";
      }));
    }
    for (auto &result : job_results) {
      result.get();
    }
    ASSERT_EQ(result, 10);
  }
  {
    int result = 0;
    auto job_results = std::vector<std::future<void>>();
    for (int i = 0; i < 20; i++) {
      job_results.push_back(std::async(std::launch::async, [&etcd, i, &result]() -> void {
        ScopedEtcdLock lock(etcd, "/test_lock");
        ASSERT_EQ(true, lock.IsLocked());
        result++;
      }));
    }
    for (auto &result : job_results) {
      result.get();
    }
    ASSERT_EQ(result, 20);
  }
  delete etcd;
}

#endif

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
