#pragma once

#include "export.h"
#include "message/message_buffer.h"
#include "message/message_callback.h"
// #include "modules/app_base.h"
#include <glog/logging.h>

class AppBase;

class MYLIB_EXPORT MultuiSensorFusion {
public:
  void ProcessData(MessageBase::SPtr frame) {
    buffer_.Append(frame);
    if (io_.reader_.contains(frame->channel_name_)) {
      for (auto& cbk : io_.reader_[frame->channel_name_]) { cbk(frame); }
    }
  }

  TotalBuffer const& get_buffer() const { return buffer_; }

  template <typename _Module>
  _Module::SPtr CreateModule() {
    auto module = std::make_shared<_Module>();
    module->InitModule(this);
    modules_.push_back(module);
    return module;
  }

  MesageIO* io() { return &io_; }
  std::deque<std::shared_ptr<AppBase>>* modules() { return &modules_; }

protected:
  TotalBuffer buffer_;
  std::deque<std::shared_ptr<AppBase>> modules_;
  MesageIO io_;
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