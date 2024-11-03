#pragma once
#include <message/message_buffer.h>

class ReleativeLoc {
public:
  ReleativeLoc();

  Message<State>::SPtr ProcessImu(Message<Imu>::SCPtr frame);
  void ProcessGnss(std::shared_ptr<const Message<Gnss>> frame);
  void ProcessInitState(std::shared_ptr<const Message<State>> frame);

private:
};