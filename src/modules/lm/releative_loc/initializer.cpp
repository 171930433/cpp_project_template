#include "releative_loc/initializer.h"
namespace lm {

bool Initializer::TryInit(MessageBase::SCPtr frame) {
  if (frame->channel_type_ != ylt::reflection::get_struct_name<State>()) { return false; }

  return true;
}
void Initializer::InitByState(std::shared_ptr<const Message<State>> frame) {}
}