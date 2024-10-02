#pragma once
#include "config/channel_config.h"
#include "message/message.h"
#include <taskflow/taskflow.hpp>

class Dispatcher {
  using CallBackT = std::deque<std::function<void(MessageBase::SCPtr)>>;

public:
  void Init(tf::Executor* executor) { executor_ = executor; }

  // for app user
  template <typename _InnerStruct, typename _Module>
  void RegisterReader(std::string_view cn, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module);

  template <typename _InnerStruct, typename _Module>
  void RegisterReader(ChannelConfig const& cc, void (_Module::*mf)(std::shared_ptr<_InnerStruct>), _Module* module) {
    RegisterReader(cc.channel_name_, mf, module);
  }

  void ForeachReaders(MessageBase::SCPtr frame) { true ? SyncReaders(frame) : ASyncReaders(frame); }
  void SyncReaders(MessageBase::SCPtr frame);
  void ASyncReaders(MessageBase::SCPtr frame);

  void ForeachWriters(MessageBase::SCPtr frame) { true ? SyncWriters(frame) : ASyncWriters(frame); }
  void SyncWriters(MessageBase::SCPtr frame);
  void ASyncWriters(MessageBase::SCPtr frame);

public:
  // for msf user
  template <typename _InnerStruct>
  void RegisterWriter(std::string_view cn, std::function<void(std::shared_ptr<_InnerStruct>)>);

  template <typename _InnerStruct>
  void RegisterWriter(ChannelConfig const& cc, std::function<void(std::shared_ptr<_InnerStruct>)> func) {
    RegisterWriter(cc.channel_name_, func);
  }

public:
  std::unordered_map<std::string_view, CallBackT> reader_;
  std::unordered_map<std::string_view, CallBackT> writer_;

protected:
  tf::Executor* executor_;
  // tf::Taskflow taskflow_;
};

inline void Dispatcher::SyncReaders(MessageBase::SCPtr frame) {
  if (!reader_.contains(frame->channel_name_)) return;
  for (auto& cbk : reader_[frame->channel_name_]) { cbk(frame); }
}

inline void Dispatcher::ASyncReaders(MessageBase::SCPtr frame) {
  if (!reader_.contains(frame->channel_name_)) return;
  tf::Taskflow taskflow;

  for (auto& cbk : reader_[frame->channel_name_]) {
    std::string const task_name = std::string("read_task: ").append(frame->channel_name_);
    taskflow.emplace([cbk, frame]() { cbk(frame); }).name(task_name);
  }

  executor_->run(taskflow);
}

inline void Dispatcher::SyncWriters(MessageBase::SCPtr frame) {
  if (!writer_.contains(frame->channel_name_)) return;
  for (auto& cbk : writer_[frame->channel_name_]) { cbk(frame); }
}
inline void Dispatcher::ASyncWriters(MessageBase::SCPtr frame) {
  if (!writer_.contains(frame->channel_name_)) return;
  tf::Taskflow taskflow;

  for (auto& cbk : writer_[frame->channel_name_]) {
    std::string const task_name = std::string("write_task: ").append(frame->channel_name_);
    taskflow.emplace([cbk, frame]() {}).name(task_name);
  }

  executor_->run(taskflow);
}

template <typename _InnerStruct, typename _Module>
inline void Dispatcher::RegisterReader(
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
inline void Dispatcher::RegisterWriter(std::string_view cn, std::function<void(std::shared_ptr<_InnerStruct>)> func) {

  auto type_erased_cbk = [func](MessageBase::SCPtr message) {
    std::shared_ptr<_InnerStruct> frame = std::dynamic_pointer_cast<_InnerStruct>(message);
    assert(frame != nullptr); // 确保消息类型正确
    func(frame);
  };

  writer_[cn].push_back(type_erased_cbk);
}