#include <gtest/gtest.h>

#include "common/utils.h"

namespace pyc {

TEST(JoinHostPortTest, Ipv4AndHostname) {
    EXPECT_EQ(JoinHostPort("127.0.0.1", "8080"), "127.0.0.1:8080");
    EXPECT_EQ(JoinHostPort("status-server.svc.cluster.local", "50051"), "status-server.svc.cluster.local:50051");
}

TEST(JoinHostPortTest, Ipv6LiteralGetsBracketed) {
    EXPECT_EQ(JoinHostPort("::1", "8080"), "[::1]:8080");
    EXPECT_EQ(JoinHostPort("fe80::1", "50051"), "[fe80::1]:50051");
    EXPECT_EQ(JoinHostPort("::", "8080"), "[::]:8080");
}

TEST(JoinHostPortTest, AlreadyBracketedHostIsNotDoubleWrapped) {
    EXPECT_EQ(JoinHostPort("[::1]", "8080"), "[::1]:8080");
    EXPECT_EQ(JoinHostPort("[fe80::1]", "50051"), "[fe80::1]:50051");
}

TEST(JoinHostPortTest, EmptyHost) { EXPECT_EQ(JoinHostPort("", "8080"), ":8080"); }

}  // namespace pyc
