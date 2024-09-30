#pragma once
#include "message/message_buffer.h"
#include "message/message_callback.h"
#include "mylib.h"

class AppBase {
public:
  using SPtr = std::shared_ptr<AppBase>;

  void InitModule(MultuiSensorFusion* msf) {
    msf_ = msf;
    Init();
  }
  virtual void Init() = 0;
  virtual ~AppBase() = default;

  template <typename _InnerStruct, typename _Module>
  void RegisterReader(std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module);

  template <typename _InnerStruct>
  void WriteMessage(std::shared_ptr<_InnerStruct> frame) {
    // 先给所有订阅的module一份
    if (msf_->io_.reader_.contains(frame->channel_name_)) {
      for (auto& cbk : msf_->io_.reader_[frame->channel_name_]) { cbk->onMessage(frame); }
    }
    // 调用外部注册的函数
    if (msf_->io_.writer_.contains(frame->channel_name_)) {
      for (auto& cbk : msf_->io_.writer_[frame->channel_name_]) { cbk->onMessage(frame); }
    }
  }

  std::string name_ = "";

protected:
  MultuiSensorFusion* msf_;
};

template <typename _InnerStruct, typename _Module>
inline void AppBase::RegisterReader(
  std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module) {

  auto type_erased_cbk = [&](MessageBase::SCPtr message) {
    std::shared_ptr<_InnerStruct> frame = std::dynamic_pointer_cast<_InnerStruct>(message);
    assert(frame != nullptr); // 确保消息类型正确
    (module->*mf)(frame);
  };

  msf_->io_.reader2_[cn].push_back(type_erased_cbk);
}
