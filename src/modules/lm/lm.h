#pragma once
#include "modules/app_base.h"

class ReleativeLoc;

class LM : public AppBase {
public:
  LM();

  void Init() override {
    ELOGI << "PsinsApp init done";

    dispatcher()->RegisterReader("/imu", &LM::ProcessImu, this);
    dispatcher()->RegisterReader("/gnss", &LM::ProcessGnss, this);
    dispatcher()->RegisterReader("/init_state", &LM::ProcessInitState, this);
  }
  void ProcessImu(std::shared_ptr<const Message<Imu>> frame);
  void ProcessGnss(std::shared_ptr<const Message<Gnss>> frame);
  void ProcessInitState(std::shared_ptr<const Message<State>> frame);

private:
  std::unique_ptr<ReleativeLoc> rl_;
};