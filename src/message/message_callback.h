#pragma once
#include "message/message.h"

class MesageIO {
  using CallBackT = std::deque<std::function<void(MessageBase::SCPtr)>>;

public:
  // for app user
  template <typename _InnerStruct, typename _Module>
  void RegisterReader(std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module);

public:
  // for msf user
  template <typename _InnerStruct>
  void RegisterWriter(std::string_view cn, std::function<void(std::shared_ptr<_InnerStruct>)>);

public:
  std::unordered_map<std::string_view, CallBackT> reader_;
  std::unordered_map<std::string_view, CallBackT> writer_;
};

template <typename _InnerStruct, typename _Module>
inline void MesageIO::RegisterReader(
  std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module) {

  //! 需要按值拷贝走两个指针
  auto type_erased_cbk = [mf, module](MessageBase::SCPtr message) {
    std::shared_ptr<_InnerStruct> frame = std::dynamic_pointer_cast<_InnerStruct>(message);
    assert(frame != nullptr); // 确保消息类型正确
    (module->*mf)(frame);
  };

  reader_[cn].push_back(type_erased_cbk);
}

template <typename _InnerStruct>
inline void MesageIO::RegisterWriter(std::string_view cn, std::function<void(std::shared_ptr<_InnerStruct>)> func) {

  auto type_erased_cbk = [func](MessageBase::SCPtr message) {
    std::shared_ptr<_InnerStruct> frame = std::dynamic_pointer_cast<_InnerStruct>(message);
    assert(frame != nullptr); // 确保消息类型正确
    func(frame);
  };

  writer_[cn].push_back(type_erased_cbk);
}