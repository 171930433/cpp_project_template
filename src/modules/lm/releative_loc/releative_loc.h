#pragma once
#include "kalman.h"
#include "releative_loc/initializer.h"
#include <message/message_buffer.h>

namespace lm {

class ReleativeLoc {
public:
  ReleativeLoc();

  template <typename _Sensor>
  auto ProcessData(std::shared_ptr<Message<_Sensor> const> frame) {
    buffers_.Append(frame);
    if (!inited_) {
      inited_.store(TryInit(frame));
      if (inited_) { return; }
    }

    return ProcessImpl(frame);
  }

  Message<State>::SPtr ProcessImpl(Message<Imu>::SCPtr frame);
  //   void ProcessImpl(MessageBase::SCPtr frame) {};
  void ProcessImpl(std::shared_ptr<Message<Gnss> const> frame);
  void ProcessImpl(std::shared_ptr<Message<State> const> frame);
  bool TryInit(MessageBase::SCPtr frame);
  bool StaticCheck(Message<Imu> const& frame);

private:
  std::unique_ptr<Initializer> initializer_;
  filter::ESKF15 eskf_;

private:
  TotalBuffer<Imu, Gnss, State> buffers_;
  std::atomic_bool inited_{ false };
};

}