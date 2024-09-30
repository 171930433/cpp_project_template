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

  std::string name_ = "";

protected:
  MultuiSensorFusion* msf_;
};

template <typename _InnerStruct, typename _Module>
inline void AppBase::RegisterReader(
  std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module) {
  auto process_func = std::bind(mf, module, std::placeholders::_1);
  auto cbk = std::make_shared<MessageCallbackWithT<_InnerStruct>>(process_func);
  msf_->io_.reader_[cn].push_back(cbk);
}
