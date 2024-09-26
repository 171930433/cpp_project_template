#include "channel_message.h"

std::unordered_set<std::string> MessageBase::channel_names_ = {};
std::unordered_set<std::string_view> MessageBase::channel_types_ = {};