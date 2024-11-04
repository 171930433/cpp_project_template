#pragma once
#include "modules/app_base.h"

namespace lm {

class ReleativeLoc;

class LM : public AppBase {
public:
  LM();

  void Init() override {
    ELOGI << "lm init done";

    dispatcher()->RegisterReader("/imu", &LM::ProcessImu, this);
    dispatcher()->RegisterReader("/gnss", &LM::ProcessGnss, this);
    dispatcher()->RegisterReader("/init_state", &LM::ProcessInitState, this);
  }
  void ProcessImu(std::shared_ptr<const Message<Imu>> frame);
  void ProcessGnss(std::shared_ptr<const Message<Gnss>> frame);
  void ProcessInitState(std::shared_ptr<const Message<State>> frame);

private:
  std::shared_ptr<ReleativeLoc> rl_;
};

}
