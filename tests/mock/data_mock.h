#include "data/reader.h"
#include <gmock/gmock.h>

class MockDataReader : public IDataReader {
public:
  MOCK_METHOD((std::pair<MessageBase::SPtr, IOState>), ReadFrame, (), (override));
};