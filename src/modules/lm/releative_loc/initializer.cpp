#include "releative_loc/initializer.h"
namespace lm {

bool Initializer::TryInit(MessageBase::SCPtr frame) { return true; }
void Initializer::InitByState(std::shared_ptr<const Message<State>> frame) {}
}