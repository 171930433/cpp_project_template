#pragma once
#include "message/message.h"
#include <boost/core/noncopyable.hpp>

class MessageCallback : boost::noncopyable {
public:
  using SPtr = std::shared_ptr<MessageCallback>;
  virtual ~MessageCallback() {};
  virtual void onMessage(MessageBase::SCPtr frame) const { LOG(ERROR) << "MessageCallback::onMessage called"; };

protected:
  std::string thread_name_;
};

using MessageCallbackBuffer = std::unordered_map<std::string_view, std::deque<MessageCallback::SPtr>>;

template <typename _T>
class MessageCallbackWithT : public MessageCallback {
public:
  using CallbackType = std::function<void(std::shared_ptr<_T>)>;

  MessageCallbackWithT(const CallbackType& callback, std::string const& thread_name = "")
    : callback_(callback) {
    thread_name_ = thread_name;
  }

  // 由成员函数构造
  template <typename _Module>
  MessageCallbackWithT(void (_Module::*mf)(std::shared_ptr<_T>), _Module* module, std::string const& thread_name = "") {
    callback_ = std::bind(mf, module, std::placeholders::_1);

    thread_name_ = thread_name;
  }

  void onMessage(MessageBase::SCPtr message) const override {
    std::shared_ptr<_T> frame = std::dynamic_pointer_cast<_T>(message);
    assert(frame != nullptr); // 确保消息类型正确
    callback_(frame);
  }

protected:
  CallbackType callback_;
};

class MesageIO {
public:
  MessageCallbackBuffer reader_;
  MessageCallbackBuffer writer_;
};