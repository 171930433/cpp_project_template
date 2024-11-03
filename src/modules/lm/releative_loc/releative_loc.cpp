#include "releative_loc/releative_loc.h"

namespace lm {

ReleativeLoc::ReleativeLoc() {}

Message<State>::SPtr ReleativeLoc::ProcessImu(Message<Imu>::SCPtr frame) {
  auto re = CreateMessage<State>("/releative_loc/pose");
  return re;
}

void ReleativeLoc::ProcessGnss(std::shared_ptr<const Message<Gnss>> frame) {}
void ReleativeLoc::ProcessInitState(std::shared_ptr<const Message<State>> frame) {}

}