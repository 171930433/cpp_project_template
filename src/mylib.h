#pragma once

#include "config/config_manager.h"
#include "export.h"
#include "message/message_buffer.h"
#include "message/message_dispatcher.h"
#include <gflags/gflags.h>
// #include <glog/logging.h>
#include <taskflow/taskflow.hpp>
#include <ylt/easylog.hpp>

DECLARE_string(config_dir);
DECLARE_string(data_dir);

class AppBase;

class MYLIB_EXPORT MultuiSensorFusion {
public:
  MultuiSensorFusion() = default;
  MultuiSensorFusion(std::string const& path);
  void Init(std::string const& path = FLAGS_config_dir);

public:
  void ProcessData(MessageBase::SPtr frame) {
    buffer_.Append(frame);
    dispatcher_.ForeachReaders(frame);
  }

  template <typename _Module>
  _Module::SPtr CreateModule() {
    auto module = std::make_shared<_Module>();
    module->InitModule(this);
    modules_.push_back(module);
    return module;
  }

  TotalBuffer const* get_buffer() const { return &buffer_; }
  Dispatcher* dispatcher() { return &dispatcher_; }
  ConfigManager* cm() { return &cm_; }
  std::deque<std::shared_ptr<AppBase>> const* modules() const { return &modules_; }

public:
  TotalBuffer buffer_;
  std::deque<std::shared_ptr<AppBase>> modules_;
  Dispatcher dispatcher_;
  ConfigManager cm_;
  tf::Executor executor_;
  //
  friend AppBase;
};

#ifdef HAVE_THIRD_PARTY
#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>
#endif

int MYLIB_EXPORT add(int const x, int const y);
int MYLIB_NO_EXPORT minus(int const x, int const y);

#ifdef HAVE_THIRD_PARTY

template <typename _T>
inline std::string to_string(_T const& elem) {
  std::string str;
  struct_json::to_json(elem, str); // {"name":"tom","age":20}
  return str;
}

#endif