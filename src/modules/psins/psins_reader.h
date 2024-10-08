#pragma once
#include "PSINSCore/PSINS.h"
#include "data/reader.h"
#include <deque>

namespace convert {

inline Vec3d ToVec3d(CVect3 const& in) {
  return { in.i, in.j, in.k };
}
inline CVect3 ToCVect3(Vec3d const& in) {
  return { in.x_, in.y_, in.z_ };
}

inline Message<Imu>::SPtr ToImu(DataSensor* frame) {
  auto imu = CreateMessage<Imu>("/imu");
  imu->msg_.acc_ = convert::ToVec3d(frame->vm);
  imu->msg_.gyr_ = convert::ToVec3d(frame->wm);
  imu->msg_.t0_ = frame->t;
  return imu;
}

inline Message<Gnss>::SPtr ToGnss(DataSensor* frame) {
  auto gnss = CreateMessage<Gnss>("/gnss");
  gnss->msg_.t0_ = frame->t;
  gnss->msg_.pos_.pos = convert::ToVec3d(frame->gpspos);
  gnss->msg_.vel_.vel = convert::ToVec3d(frame->gpsvn);
  gnss->UpdateRelativePose();
  return gnss;
}

inline Message<State>::SPtr ToState(DataSensor* frame) {
  auto state = CreateMessage<State>("/init_state");
  state->msg_.t0_ = frame->t;
  state->msg_.pos_ = convert::ToVec3d(frame->gpspos);
  state->msg_.vel_ = convert::ToVec3d(frame->gpsvn);
  state->msg_.att_ = convert::ToVec3d(frame->att);
  return state;
}

inline Message<State>::SPtr ToState(CSINS const& frame) {
  auto state = CreateMessage<State>("/fused_state");
  state->msg_.t0_ = frame.tk;
  state->msg_.pos_ = convert::ToVec3d(frame.pos);
  state->msg_.vel_ = convert::ToVec3d(frame.vn);
  state->msg_.att_ = convert::ToVec3d(frame.att);
  state->UpdateRelativePose();
  return state;
}

}

class PsinsReader : public IDataReader {
public:
  void Init(std::string const& path) override {
    reader_ = std::make_unique<CFileRdSr>(path.c_str());

    // 发送初始状态消息
    buffer_.push_back({ convert::ToState(&reader_->DS0), IOState::OK });
  }
  std::pair<MessageBase::SPtr, IOState> ReadFrame() override;

private:
  std::unique_ptr<CFileRdSr> reader_;
  std::deque<std::pair<MessageBase::SPtr, IDataReader::IOState>> buffer_;
};

inline std::pair<MessageBase::SPtr, IDataReader::IOState> PsinsReader::ReadFrame() {
  if (!buffer_.empty()) {
    auto re = buffer_.front();
    buffer_.pop_front();
    return re;
  }

  IOState state = reader_->load(1) ? IOState::OK : IOState::END;
  if (state == IOState::END) return { nullptr, state };

  // imu
  DataSensor* frame = (DataSensor*)reader_->buff;
  auto imu = convert::ToImu(frame);

  //   gnss
  if (frame->gpspos.i > 0.1) {
    auto gnss = convert::ToGnss(frame);

    buffer_.push_back({ gnss, IOState::OK });
  }

  return { imu, state };
}
