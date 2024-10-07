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
  // msg_str += ",origin_:";
  // iguana::to_json(origin_, msg_str);
  // msg_str += ",pos_xyz_:";
  // iguana::to_json(pos_xyz_, msg_str);
  msg_str += ",msg_:";
  iguana::to_json(this->msg_, msg_str);

  return msg_str;
}

template <typename _Sensor>
void Message<_Sensor, true>::UpdateRelativePose() {
  static std::unordered_map<std::string_view, Eigen::Isometry3d> origins;
  static std::unordered_map<std::string_view, WGS84> wgs84_ellipsoids;

  Eigen::Isometry3d const Twb = ToIsometry3d(this->msg_);
  WGS84& ellipsoid = wgs84_ellipsoids[this->channel_name_];

  if (!origins.contains(this->channel_name_)) {
    origins[this->channel_name_] = Twb;
    ellipsoid.SetOrigin(Twb.translation());
  }

  origin_ = &origins[this->channel_name_];

  rpose_.linear() = origin_->linear() * Twb.linear().inverse();
  rpose_.translation() = ellipsoid.LLH2ENU(Twb.translation());
}
