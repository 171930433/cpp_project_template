template <typename _Message>
Channel<_Message>::Channel(std::string const& channel_name) {
  static std::unordered_set<std::string> channel_names;
  auto const& [it, insert_re] = channel_names.insert(channel_name);
  channel_name_ = *it;
  channel_type_ = ylt::reflection::type_string<_Message>();
}

template <typename _Message>
std::shared_ptr<Channel<_Message>> Channel<_Message>::Create(
  std::string const& channel_name) {
  return std::make_shared<Channel>(channel_name);
}

template <typename _Message>
inline std::string Channel<_Message>::to_json() const {
  std::string msg_str = this->to_header_str();
  msg_str += ",msg_:";
  iguana::to_json(msg_, msg_str);
  return msg_str;
}