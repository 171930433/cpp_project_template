#pragma once
#include "message/message.h"
#include <boost/core/noncopyable.hpp>

class ZFrameCallback : boost::noncopyable {

public:
  virtual ~ZFrameCallback() {};
  virtual void onMessage(MessageBase::SCPtr frame) const { LOG(ERROR) << "ZFrameCallback::onMessage called"; };

protected:
  std::string thread_name_;
};

template <typename _T>
class ZFrameCallbackWithT : public ZFrameCallback {
public:
  using CallbackType = std::function<void(std::string const&, std::shared_ptr<_T>)>;

  ZFrameCallbackWithT(const CallbackType& callback, std::string const& thread_name = "")
    : callback_(callback) {
    thread_name_ = thread_name;
  }

  // 由成员函数构造
  template <typename _ModuleType>
  ZFrameCallbackWithT(void (_ModuleType::*mf)(std::string const&, std::shared_ptr<_T>), _ModuleType* module,
    std::string const& thread_name = "") {
    callback_ = std::bind(mf, module, std::placeholders::_1, std::placeholders::_2);

    thread_name_ = thread_name;
  }

  void onMessage(MessageBase::SCPtr frame) const override {
    std::shared_ptr<_T> frame = std::dynamic_pointer_cast<_T>(message);
    assert(frame != nullptr); // 确保消息类型正确
    callback_(channel_name, frame);
  }

protected:
  CallbackType callback_;
};