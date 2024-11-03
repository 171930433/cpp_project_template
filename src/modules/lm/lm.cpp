#include "lm.h"
#include <taskflow/taskflow.hpp>

#include "releative_loc/releative_loc.h"

LM::LM() { rl_ = std::make_unique<ReleativeLoc>(); }

void LM::ProcessImu(std::shared_ptr<const Message<Imu>> frame) {
  tf::Taskflow tf;
  tf.emplace([this, frame]() {
      auto re = rl_->ProcessImu(frame);
      if (re) { this->WriteMessage(re); }
    })
    .name("ReleativeLoc ProcessImu");

  executor()->run(tf);
}
void LM::ProcessGnss(std::shared_ptr<const Message<Gnss>> frame) {
  tf::Taskflow tf;

  tf.emplace([this, frame]() { rl_->ProcessGnss(frame); }).name("ReleativeLoc ProcessGnss");

  executor()->run(tf);
}
void LM::ProcessInitState(std::shared_ptr<const Message<State>> frame) {
  tf::Taskflow tf;

  tf.emplace([this, frame]() { rl_->ProcessInitState(frame); }).name("ReleativeLoc ProcessImu");

  executor()->run(tf);
}