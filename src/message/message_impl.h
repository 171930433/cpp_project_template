template <typename _Sensor>
Message<_Sensor, false>::Message(std::string const& channel_name) {
  static std::unordered_set<std::string> channel_names;
  auto const& [it, insert_re] = channel_names.insert(channel_name);
  channel_name_ = *it;
  channel_type_ = ylt::reflection::type_string<_Sensor>();
}

template <typename _Sensor>
inline std::string Message<_Sensor, false>::to_json() const {
  std::string msg_str = this->to_header_str();
  msg_str += ",msg_:";
  iguana::to_json(msg_, msg_str);
  return msg_str;
}

template <typename _Sensor>
inline std::string Message<_Sensor, true>::to_json() const {
  std::string msg_str = this->to_header_str();
  if (origin_) {
    msg_str += ",origin_:";
    iguana::to_json(*origin_, msg_str);
  } else {
    msg_str += ",origin_:null";
  }
  msg_str += ",rpose_:";
  iguana::to_json(rpose_, msg_str);
  msg_str += ",msg_:";
  iguana::to_json(this->msg_, msg_str);

  return msg_str;
}

template <typename _Sensor>
void Message<_Sensor, true>::UpdateRelativePose(bool is_llh_pose) {
  static std::unordered_map<std::string_view, Eigen::Isometry3d> origins;
  static std::unordered_map<std::string_view, WGS84> wgs84_ellipsoids;

  Eigen::Isometry3d const Twb = ToIsometry3d(this->msg_);
  WGS84& ellipsoid = wgs84_ellipsoids[this->channel_name_];

  // 确定圆心
  if (!origins.contains(this->channel_name_)) {
    if (is_llh_pose) {
      origins[this->channel_name_] = Twb;
      ellipsoid.SetOrigin(Twb.translation());
    } else {
      origins[this->channel_name_].setIdentity();
      ellipsoid.SetOrigin(Eigen::Vector3d::Zero());
    }
  }

  // 确定相对位置
  origin_ = &origins[this->channel_name_];

  if (is_llh_pose) {
    rpose_.linear() = origin_->linear().inverse() * Twb.linear();
    rpose_.translation() = ellipsoid.LLH2ENU(Twb.translation());
  } else {
    rpose_ = Twb;
  }
}
