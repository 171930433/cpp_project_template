#pragma once

#include "export.h"
#include "message/message_buffer.h"

class MYLIB_EXPORT MultuiSensorFusion {
public:
  template <typename _Message>
  void Append(std::shared_ptr<ChannelMsg<_Message>> frame) {
    buffer_.Append(frame);
  }

  TotalBuffer const& get_buffer() const { return buffer_; }

protected:
  TotalBuffer buffer_;
};

#ifdef HAVE_THIRD_PARTY
#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>
#endif

int MYLIB_EXPORT add(int const x, int const y);
int MYLIB_NO_EXPORT minus(int const x, int const y);

#ifdef HAVE_THIRD_PARTY

template <typename _T>
inline std::string to_string(_T const& elem) {
  std::string str;
  struct_json::to_json(elem, str); // {"name":"tom","age":20}
  return str;
}

#endif