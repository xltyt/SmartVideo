/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.07 17:44:32
 *
\****************************************************/

#include "sqlite_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <glog/logging.h>
#include "string_utils.h"
#include "timer.h"

using namespace mycommon;

int SqliteUtils::ResultSet::get_data(size_t i, size_t j, std::string& info) const {
  if (i >= _vec_result.size() || j >= _field) {
    return -1;
  }
  info = _vec_result[i][j];
  return 0;
}
      
std::string SqliteUtils::ResultSet::get_data(size_t i, size_t j) const {
  if (i >= _vec_result.size() || j >= _field) {
    return "";
  }
  return _vec_result[i][j];
}
      
int SqliteUtils::ResultSet::row(int i, std::vector<std::string>& info) const {
  if (i < 0 || i >= (int)_vec_result.size()) {
    return -1;
  }
  info = _vec_result[i];
  return 0;
}
      
std::string SqliteUtils::ResultSetMap::get_field(const std::unordered_map<std::string, std::string>& maps, const std::string& field, const std::string& default_val /*= ""*/) const {
  std::unordered_map<std::string, std::string>::const_iterator iter = maps.find(field);
  if (iter != maps.end()) {
    return iter->second;
  }
  LOG(WARNING) << "SqliteUtils::ResultSetMap::get_field Cant Find Field[" << field << "], Use Default Value";
  return default_val;
}
      
int SqliteUtils::ResultSetMap::get_field_int(const std::unordered_map<std::string, std::string>& maps, const std::string& field, int default_val /*= -1*/) const {
  std::unordered_map<std::string, std::string>::const_iterator iter = maps.find(field);
  if (iter != maps.end()) {
    return atoi(iter->second.c_str());
  }
  LOG(WARNING) << "SqliteUtils::ResultSetMap::get_field_int Cant Find Field[" << field << "], Use Default Value";
  return default_val;
}
      
int64_t SqliteUtils::ResultSetMap::get_field_bigint(const std::unordered_map<std::string, std::string>& maps, const std::string& field, int64_t default_val /*= -1*/) const {
  std::unordered_map<std::string, std::string>::const_iterator iter = maps.find(field);
  if (iter != maps.end()) {
    return strtoll(iter->second.c_str(), NULL, 10);
  }
  LOG(WARNING) << "SqliteUtils::ResultSetMap::get_field_bigint Cant Find Field[" << field << "], Use Default Value";
  return default_val;
}

SqliteUtils::SqliteUtils(const std::string& path) {
  _sql = NULL;
  _path = path;
}

SqliteUtils::~SqliteUtils() {
  close();
}

void SqliteUtils::close() {
  if (NULL != _sql) {
    try {
      _sql->close();
    }
    catch (CppSQLite3Exception e) {
      LOG(ERROR) << "SqliteUtils::close Failed[" << e.errorMessage() << "]";
    }
    delete _sql;
    _sql = NULL;
  }
}
    
int SqliteUtils::connect() {
  close();
  int ret = -1;
  _sql = new CppSQLite3DB();
  try {
    _sql->open(_path.c_str());
    ret = 0;
  }
  catch (CppSQLite3Exception e) {
    delete _sql;
    _sql = NULL;
    LOG(ERROR) << "SqliteUtils::connect Connect Failed[" << e.errorMessage() << "]";
  }
  return ret;
}
    
int SqliteUtils::execute(const char *sql, bool enable_log /*= true*/) {
  uint64_t start_time = mycommon::getMilliTime();
  common::ScopedLockW<common::RWLock> lock(_lock);
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      LOG(ERROR) << "SqliteUtils::execute Failed, Sql[" << sql << "] Connect Failed";
      return -1; 
    }
  }
  int rows = 0;
  try {
    rows = _sql->execDML(sql);
  }
  catch (CppSQLite3Exception e) {
    LOG(ERROR) << "SqliteUtils::execute Failed, Sql[" << sql << "] Msg[" << e.errorMessage() << "]";
    return -1;
  }
  if (enable_log) LOG(INFO) << "SqliteUtils::execute SQL[" << sql << "] Affect[" << rows << "] Time[" << (mycommon::getMilliTime() - start_time) << "]";
  return rows;
}

int SqliteUtils::execute(const std::string& sql, bool enable_log /*= true*/) {
  return execute(sql.c_str(), enable_log);
}
    
int SqliteUtils::execute(const std::vector<std::string>& sqls_ori, bool enable_log /*= true*/) {
  if (0 == sqls_ori.size()) {
    return 0;
  }
  uint64_t start_time = mycommon::getMilliTime();
  common::ScopedLockW<common::RWLock> lock(_lock);
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      LOG(ERROR) << "SqliteUtils::execute Failed, Sql[" << mycommon::str_join(sqls_ori) << "] Connect Failed";
      return -1; 
    }
  }
  std::vector<std::string> sqls;
  sqls.push_back("BEGIN;");
  for (auto &_ : sqls_ori) {
    sqls.push_back(_);
  }

  int affect_total = 0;
  int ret = 0;
  for (auto &sql : sqls) {
    int affect = 0;
    ret = execute_internal(sql.c_str(), &affect);
    if (0 != ret) {
      execute_internal("ROLLBACK;");
      return ret;
    }
    affect_total += affect;
  }
  execute_internal("COMMIT;");

  if (enable_log) LOG(INFO) << "SqliteUtils::execute SQL[" << mycommon::str_join(sqls_ori) << "] Affect[" << affect_total << "] Time[" << (mycommon::getMilliTime() - start_time) << "]";
  return affect_total;
}
    
int SqliteUtils::execute(const std::string& sql, const std::vector<std::string>& vals, int batch_count /*= 200*/, bool enable_log /*= true*/) {
  int rows = 0;
  std::vector<std::string> sub_vals;
  for (int i = 0; i < (int)vals.size(); i++) {
    sub_vals.push_back(vals[i]);
    if (sub_vals.size() >= batch_count) {
      std::string sql_ful = sql + mycommon::str_join(sub_vals, ",");
      sub_vals.clear();
      int ret = execute(sql_ful.c_str(), enable_log);
      if (ret < 0) {
        return ret;
      }
      rows += ret;
    }
  }
  if (sub_vals.size()) {
    std::string sql_ful = sql + mycommon::str_join(sub_vals, ",");
    int ret = execute(sql_ful.c_str(), enable_log);
    if (ret < 0) {
      return ret;
    }
    rows += ret;
  }
  return rows;
}

int SqliteUtils::execute_format(const char *format, ...) {
  uint64_t start_time = mycommon::getMilliTime();
  char sql[8192] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(sql, 8192 - 1, format, args);
  //my_vsnprintf(sql, 8192 - 1, format, args);
  va_end(args);
  common::ScopedLockW<common::RWLock> lock(_lock);
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      LOG(ERROR) << "SqliteUtils::execute_format Failed, Sql[" << sql << "] Connect Failed";
      return -1; 
    }
  }
  int rows = 0;
  try {
    rows = _sql->execDML(sql);
  }
  catch (CppSQLite3Exception e) {
    LOG(ERROR) << "SqliteUtils::execute_format Failed, Sql[" << sql << "] Msg[" << e.errorMessage() << "]";
    return -1;
  }
  LOG(INFO) << "SqliteUtils::execute_format SQL[" << sql << "] Affect[" << rows << "] Time[" << (mycommon::getMilliTime() - start_time) << "]";
  return rows;
}

int SqliteUtils::fetch(ResultSet& info, const char *sql) {
  uint64_t start_time = mycommon::getMilliTime();
  common::ScopedLockW<common::RWLock> lock(_lock);
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      LOG(ERROR) << "SqliteUtils::fetch Failed, Sql[" << sql << "] Connect Failed";
      return -1; 
    }   
  }
  try {
    CppSQLite3Query query = _sql->execQuery(sql);
    int num_fields = query.numFields();
    info._field = num_fields;
    while (!query.eof()) {
      std::vector<std::string> vec_string;
      for (int j = 0; j < num_fields; j++) {
        const char* val = query.fieldValue(j);
        if (val != NULL) {
          vec_string.push_back(val);
        }
        else {
          vec_string.push_back("");
        }
      }
      info._vec_result.push_back(vec_string);
      query.nextRow();
    }
    query.finalize();
  }
  catch (CppSQLite3Exception e) {
    LOG(ERROR) << "SqliteUtils::fetch Failed, Sql[" << sql << "] Msg[" << e.errorMessage() << "]";
    return -1;
  }
  LOG(INFO) << "SqliteUtils::fetch SQL[" << sql << "] Size[" << info._vec_result.size() << "] Time[" << (mycommon::getMilliTime() - start_time) << "]";
  return info._vec_result.size();
}
    
int SqliteUtils::fetch_format(ResultSet& info, const char *format, ...) {
  char sql[8192] = {0};
  va_list args;
  va_start(args, format);
  //my_vsnprintf(NULL, sql, 8192 - 1, format, args);
  vsnprintf(sql, 8192 - 1, format, args);
  va_end(args);
  return fetch(info, sql);
}

int SqliteUtils::fetch(ResultSetMap& info, const char *sql) {
  uint64_t start_time = mycommon::getMilliTime();
  common::ScopedLockW<common::RWLock> lock(_lock);
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      LOG(ERROR) << "SqliteUtils::fetch Failed, Sql[" << sql << "] Connect Failed";
      return -1; 
    }   
  }
  try {
    CppSQLite3Query query = _sql->execQuery(sql);
    int num_fields = query.numFields();
    info._field = num_fields;
    const char *headers[info._field];
    for (int j = 0; j < num_fields; j++) {
      const char* val = query.fieldName(j);
      ResultSetMap::HEADER_INFO header_info;
      header_info.name = (NULL != val ? val : "");
      info._vec_header.push_back(header_info);
      headers[j] = val;
    }
    while (!query.eof()) {
      std::unordered_map<std::string, std::string> row_map;
      for (int j = 0; j < num_fields; j++) {
        const char* val = query.fieldValue(j);
        if (val != NULL) {
          row_map[headers[j]] = val;
        }
        else {
          row_map[headers[j]] = "";
        }
      }
      info._vec_result.push_back(row_map);
      query.nextRow();
    }
    query.finalize();
  }
  catch (CppSQLite3Exception e) {
    LOG(ERROR) << "SqliteUtils::fetch Failed, Sql[" << sql << "] Msg[" << e.errorMessage() << "]";
    return -1;
  }
  LOG(INFO) << "SqliteUtils::fetch SQL[" << sql << "] Size[" << info._vec_result.size() << "] Time[" << (mycommon::getMilliTime() - start_time) << "]";
  return info._vec_result.size();
}
    
int SqliteUtils::fetch_format(ResultSetMap& info, const char *format, ...) {
  char sql[8192] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(sql, 8192 - 1, format, args);
  va_end(args);
  return fetch(info, sql);
}
    
int SqliteUtils::fetch_format_scalar(const std::string& default_val, std::string& result, const char *format, ...) {
  char sql[8192] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(sql, 8192 - 1, format, args);
  va_end(args);
  LOG(INFO) << "SqliteUtils::fetch_format_scalar SQL[" << sql << "]";
  ResultSet info;
  int ret = fetch_format(info, sql);
  if (ret < 0) {
    return ret;
  }
  if (info.row() < 1 || info.field() < 1) {
    result = default_val;
    return 0;
  }
  result = info.get_data(0, 0);
  return 0;
}

int SqliteUtils::fetch_format_scalar(int default_val, int& result, const char *format, ...) {
  char sql[8192] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(sql, 8192 - 1, format, args);
  va_end(args);
  LOG(INFO) << "SqliteUtils::fetch_format_scalar SQL[" << sql << "]";
  ResultSet info;
  int ret = fetch_format(info, sql);
  if (ret < 0) {
    return ret;
  }
  if (info.row() < 1 || info.field() < 1) {
    result = default_val;
    return 0;
  }
  std::string val = info.get_data(0, 0);
  result = atoi(val.c_str());
  return 0;
}

std::string SqliteUtils::escape(const std::string& in) {
  std::string out = in;
  mycommon::str_replace(out, "\"", "\\\"");
  return out;
}

uint64_t SqliteUtils::last_id() {
  if (NULL == _sql) {
    connect();
    if (NULL == _sql) {
      return 0; 
    }   
  }
  return _sql->lastRowId();
}

int SqliteUtils::execute_internal(const char *sql, int *affect /*= NULL*/) {
  int rows = 0;
  try {
    rows = _sql->execDML(sql);
  }
  catch (CppSQLite3Exception e) {
    LOG(ERROR) << "SqliteUtils::execute_internal Failed, Sql[" << sql << "] Msg[" << e.errorMessage() << "]";
    return -1;
  }
  if (NULL != affect) {
    *affect = rows;
  }
  return 0;
}

/* vim: set expandtab ts=2 sw=2 sts=2: */
