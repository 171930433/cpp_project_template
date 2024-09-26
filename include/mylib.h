#pragma once

#include "export.h"
#include "sensor_frames.h"
#include "channel_message.h"

// #include <reflection/member_value.hpp>
// #include <reflection/private_visitor.hpp>
// #include <reflection/template_switch.hpp>
// #include <reflection/user_reflect_macro.hpp>

#ifdef HAVE_THIRD_PARTY
#include <ylt/struct_json/json_reader.h>
#include <ylt/struct_json/json_writer.h>
#endif

int MYLIB_EXPORT add(int const x, int const y);
int MYLIB_NO_EXPORT minus(int const x, int const y);

#ifdef HAVE_THIRD_PARTY

template <typename _T>
inline std::string to_string(_T const &elem)
{
    std::string str;
    struct_json::to_json(elem, str); // {"name":"tom","age":20}
    return str;
}

#endif