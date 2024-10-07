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
  msg_str += ",origin_:";
  iguana::to_json(origin_, msg_str);
  msg_str += ",pos_xyz_:";
  iguana::to_json(pos_xyz_, msg_str);
  msg_str += ",msg_:";
  iguana::to_json(this->msg_, msg_str);

  return msg_str;
}

template <typename _Sensor>
Message<_Sensor, true>::Message(std::string const& channel_name)
  : Base(channel_name) {
  static std::unordered_map<std::string_view, Vec3d> origins;

  if (!origin_) {
    origin_ = &origins[this->channel_name_];
    if constexpr (std::is_same_v<_Sensor, Gnss>) {
      *origin_ = this->msg_.pos_.pos;
    } else {
      *origin_ = this->msg_.pos_;
    }
  }
  // convert
}