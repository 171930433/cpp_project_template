#include "lm.h"
#include <taskflow/taskflow.hpp>

#include "releative_loc/releative_loc.h"

namespace lm {

LM::LM() { rl_ = std::make_shared<ReleativeLoc>(); }

void LM::ProcessImu(std::shared_ptr<Message<Imu> const> frame) {
  // tf::Taskflow tf;
  // tf.emplace([this, frame]() {
  //     auto re = rl_->ProcessData(frame);
  //     if (re) { this->WriteMessage(re); }
  //   })
  //   .name("ReleativeLoc ProcessImu");

  // executor()->run(tf);
  auto re = rl_->ProcessData(frame);
  if (re) { this->WriteMessage(re); }
}
void LM::ProcessGnss(std::shared_ptr<const Message<Gnss>> frame) {
  // tf::Taskflow tf;

  // tf.emplace([this, frame]() { rl_->ProcessData(frame); }).name("ReleativeLoc ProcessGnss");

  // executor()->run(tf);
  rl_->ProcessData(frame);
}
void LM::ProcessInitState(std::shared_ptr<const Message<State>> frame) {
  // tf::Taskflow tf;

  // tf.emplace([this, frame]() { rl_->ProcessData(frame); }).name("ReleativeLoc ProcessInitState");

  // executor()->run(tf);
  rl_->ProcessData(frame);
}

}
