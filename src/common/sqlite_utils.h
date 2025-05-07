/****************************************************\
 *
 * Copyright (C) 2019 All Rights Reserved
 * Last modified: 2025.05.07 17:44:36
 *
\****************************************************/

#ifndef _SQLITE_UTILS_H__
#define _SQLITE_UTILS_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <CppSQLite3.h>
#include <lock.h>

namespace mycommon {
  class SqliteUtils {
  public:
    SqliteUtils(const std::string& path);
    ~SqliteUtils();

    class ResultSet {
    public:
      ResultSet() {
        _field = 0;
      }
      virtual ~ResultSet() {
      }
    public:
      int get_data(size_t i, size_t j, std::string& info) const;
      std::string get_data(size_t i, size_t j) const;
      int row(int i, std::vector<std::string>& info) const;
      size_t row() const {
        return _vec_result.size();
      }
      size_t field() const {
        return _field;
      }
    protected:
      friend class SqliteUtils;
      std::vector<std::vector<std::string> > _vec_result;
      size_t _field;
    };
    
    class ResultSetMap {
    public:
      ResultSetMap() {
        _field = 0;
      }
      virtual ~ResultSetMap() {
      }
    public:
      const std::vector<std::unordered_map<std::string, std::string> >& data() const {
        return _vec_result;
      }
      struct HEADER_INFO {
        std::string name;
      };
      const std::vector<HEADER_INFO>& header() const {
        return _vec_header;
      }
      std::string get_field(const std::unordered_map<std::string, std::string>& maps, const std::string& field, const std::string& default_val = "") const;
      int get_field_int(const std::unordered_map<std::string, std::string>& maps, const std::string& field, int default_val = -1) const;
      int64_t get_field_bigint(const std::unordered_map<std::string, std::string>& maps, const std::string& field, int64_t default_val = -1) const;
      size_t row() const {
        return _vec_result.size();
      }
      size_t field() const {
        return _field;
      }
    protected:
      friend class SqliteUtils;
      std::vector<std::unordered_map<std::string, std::string> > _vec_result;
      std::vector<HEADER_INFO> _vec_header;
      size_t _field;
    };

    void close();
    int connect();
    int execute(const char *sql, bool enable_log = true);
    int execute(const std::string& sql, bool enable_log = true);
    int execute(const std::vector<std::string>& sqls, bool enable_log = true);
    int execute(const std::string& sql, const std::vector<std::string>& vals, int batch_count = 200, bool enable_log = true);
    int execute_format(const char *format, ...);
    int fetch(ResultSet& info, const char *sql);
    int fetch_format(ResultSet& info, const char *format, ...);
    int fetch(ResultSetMap& info, const char *sql);
    int fetch_format(ResultSetMap& info, const char *format, ...);
    int fetch_format_scalar(const std::string& default_val, std::string& result, const char *format, ...);
    int fetch_format_scalar(int default_val, int& result, const char *format, ...);
    std::string escape(const std::string& in);
    uint64_t last_id();

  protected:
    CppSQLite3DB *_sql;
    std::string _path;
    std::string _db_name;
    std::string _charset;
    common::RWLock _lock;
    int execute_internal(const char *sql, int *affect = NULL);
  };
}


#endif

/* vim: set expandtab nu ts=2 sw=2 sts=2: */
