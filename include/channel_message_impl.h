template <typename _Message>
ChannelMsg<_Message>::ChannelMsg(std::string const& channel_name) {
  auto const& [it, insert_re] = channel_names_.insert(channel_name);
  channel_name_ = *it;
  channel_type_ = ylt::reflection::type_string<_Message>();
  channel_types_.insert(channel_type_);
}

template <typename _Message>
std::shared_ptr<ChannelMsg<_Message>> ChannelMsg<_Message>::Create(
  std::string const& channel_name) {
  return std::make_shared<ChannelMsg>(channel_name);
}

template <typename _Message>
std::string ChannelMsg<_Message>::to_json_str() const {
  std::string msg_str = MessageBase::to_json_str();
  msg_str += "msg_:";
  iguana::to_json(msg_, msg_str);
  return msg_str;
}