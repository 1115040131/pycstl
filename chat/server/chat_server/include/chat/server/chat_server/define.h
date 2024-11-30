#pragma once

#include <boost/asio.hpp>

#include "logger/logger.h"

namespace pyc {
namespace chat {

using tcp = boost::asio::ip::tcp;

inline Logger g_logger("ChatServer");

}  // namespace chat
}  // namespace pyc

#define PYC_LOG_DEBUG(...) ::pyc::chat::g_logger.debug(__VA_ARGS__)
#define PYC_LOG_INFO(...) ::pyc::chat::g_logger.info(__VA_ARGS__)
#define PYC_LOG_WARN(...) ::pyc::chat::g_logger.warn(__VA_ARGS__)
#define PYC_LOG_ERROR(...) ::pyc::chat::g_logger.error(__VA_ARGS__)
#define PYC_LOG_FATAL(...) ::pyc::chat::g_logger.fatal(__VA_ARGS__)
