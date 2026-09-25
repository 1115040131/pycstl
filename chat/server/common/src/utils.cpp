module;

#include <string>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

module chat.server.common.utils;

namespace pyc {
namespace chat {

std::string generateUniqueString() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

}  // namespace chat
}  // namespace pyc
