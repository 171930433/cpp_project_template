#pragma once
#include "sensor_frames.h"
#include <unordered_set>
#include <ylt/reflection/template_string.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>

struct MessageBase
{
    using SPtr = std::shared_ptr<MessageBase>; 
    using SCPtr = std::shared_ptr<MessageBase const>; 
    virtual ~MessageBase() = default;
    virtual double t0() const = 0;

public:
    std::string_view channel_name_ = "";
    std::string_view channel_type_ = "";
    double t1_ = 0; // 接收到时间
    double t2_ = 0; // 落盘时间
public:
    static std::unordered_set<std::string> channel_names_;
    static std::unordered_set<std::string_view> channel_types_;
};

template <typename _Message>
struct ChannelMsg : public MessageBase
{
    ChannelMsg(std::string const &channel_name)
    {
        auto const &[it, insert_re] = channel_names_.insert(channel_name);
        channel_name_ = *it;
        channel_type_ = ylt::reflection::type_string<_Message>();
        channel_types_.insert(channel_type_);
    }
    static std::shared_ptr<ChannelMsg> Create(std::string const &channel_name)
    {
        return std::make_shared<ChannelMsg>(channel_name);
    }
    std::string to_json_str() const
    {
        std::stringstream ss;
        ss << fmt::format(R"("channel_name_":"{}",)", channel_name_);
        ss << fmt::format(R"("channel_type_":"{}",)", channel_type_);
        ss << fmt::format(R"("t1_":{},)", t1_);
        ss << fmt::format(R"("t2_":{},)", t2_);
        std::string msg_str;
        iguana::to_json(msg_, msg_str);
        ss << "msg_:" << msg_str;
        return ss.str();
    }

    friend std::ostream &operator<<(std::ostream &os, ChannelMsg const &elem)
    {
        os << elem.to_json_str();
        return os;
    }

public:
    double t0() const override { return msg_.t0_; }
    _Message msg_;
};