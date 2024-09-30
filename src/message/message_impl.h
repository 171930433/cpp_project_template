template <typename _Sensor>
Message<_Sensor>::Message(std::string const& channel_name) {
  static std::unordered_set<std::string> channel_names;
  auto const& [it, insert_re] = channel_names.insert(channel_name);
  channel_name_ = *it;
  channel_type_ = ylt::reflection::type_string<_Sensor>();
}

template <typename _Sensor>
std::shared_ptr<Message<_Sensor>> Message<_Sensor>::Create(
  std::string const& channel_name) {
  return std::make_shared<Message>(channel_name);
}

template <typename _Sensor>
inline std::string Message<_Sensor>::to_json() const {
  std::string msg_str = this->to_header_str();
  msg_str += ",msg_:";
  iguana::to_json(msg_, msg_str);
  return msg_str;
}