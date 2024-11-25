#pragma once
#include "PSINSCore/KFApp.h"
#include "modules/app_base.h"
#include "psins_reader.h"
#include <ylt/easylog.hpp>

/*! 1. 从暴露的接口来看,预测和量测在同一线程
 */
class PsinsApp : public AppBase {
public:
  using SPtr = std::shared_ptr<PsinsApp>;
  PsinsApp() { name_ = "PsinsApp"; }

  void Init() override {
    ELOGI << "PsinsApp init done";

    dispatcher()->RegisterReader("/imu", &PsinsApp::ProcessImu, this);
    dispatcher()->RegisterReader("/gnss", &PsinsApp::ProcessGnss, this);
    dispatcher()->RegisterReader("/init_state", &PsinsApp::ProcessInitState, this);

    //
    dt_ = 1.0 / 125;
    kf_app_ = std::make_unique<CKFApp>(dt_);
    // kf_app_->Init(CSINS(pDS0->att, pDS0->gpsvn, pDS0->gpspos, pDS0->t));
  }
  void ProcessImu(std::shared_ptr<const Message<Imu>> frame);
  void ProcessGnss(std::shared_ptr<const Message<Gnss>> frame);
  void ProcessInitState(std::shared_ptr<const Message<State>> frame);

private:
  std::unique_ptr<CKFApp> kf_app_;
  std::atomic_bool inited_{ false };
  double dt_ = 0.01;
};

inline void PsinsApp::ProcessImu(std::shared_ptr<const Message<Imu>> frame) {
  if (!inited_) return;
  // ELOG_DEBUG << frame->to_header_str();

  auto acc = convert::ToCVect3(frame->msg_.acc_);
  auto gyr = convert::ToCVect3(frame->msg_.gyr_);
  kf_app_->Update(&gyr, &acc, 1, dt_);

  // 构造输出
  auto re = convert::ToState(kf_app_->sins);
  WriteMessage(re);
}
inline void PsinsApp::ProcessGnss(std::shared_ptr<const Message<Gnss>> frame) {
  if (!inited_) return;

  // ELOG_DEBUG << frame->to_header_str() << " " << frame->rpose_.translation().transpose();
  kf_app_->SetMeasGNSS(convert::ToCVect3(frame->msg_.pos_.pos), convert::ToCVect3(frame->msg_.vel_.vel));
}

inline void PsinsApp::ProcessInitState(std::shared_ptr<const Message<State>> frame) {
  ELOG_DEBUG << frame->to_json();

  CVect3 pos = convert::ToCVect3(frame->msg_.pos_);
  CVect3 vel = convert::ToCVect3(frame->msg_.vel_);
  CVect3 att = { frame->msg_.att_.y_, frame->msg_.att_.z_, frame->msg_.att_.x_ };
  kf_app_->Init(CSINS(att, vel, pos, frame->t0()));
  inited_.store(true);
}
