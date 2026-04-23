#include "hts/retry_policy.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(NoRetryPolicyTest, NeverRetries) {
    NoRetryPolicy policy;

    EXPECT_FALSE(policy.should_retry(0, "error"));
    EXPECT_FALSE(policy.should_retry(1, "error"));
    EXPECT_FALSE(policy.should_retry(100, "error"));
    EXPECT_EQ(policy.get_delay(0).count(), 0);
    EXPECT_STREQ(policy.name(), "NoRetry");
}

TEST(FixedRetryPolicyTest, RetriesUpToMax) {
    FixedRetryPolicy policy(3, std::chrono::milliseconds{100});

    EXPECT_TRUE(policy.should_retry(0, "error"));
    EXPECT_TRUE(policy.should_retry(1, "error"));
    EXPECT_TRUE(policy.should_retry(2, "error"));
    EXPECT_FALSE(policy.should_retry(3, "error"));
    EXPECT_FALSE(policy.should_retry(4, "error"));
}

TEST(FixedRetryPolicyTest, ConstantDelay) {
    FixedRetryPolicy policy(3, std::chrono::milliseconds{200});

    EXPECT_EQ(policy.get_delay(0).count(), 200);
    EXPECT_EQ(policy.get_delay(1).count(), 200);
    EXPECT_EQ(policy.get_delay(2).count(), 200);
    EXPECT_EQ(policy.get_delay(10).count(), 200);
}

TEST(ExponentialBackoffPolicyTest, RetriesUpToMax) {
    ExponentialBackoffPolicy policy(5);

    EXPECT_TRUE(policy.should_retry(0, "error"));
    EXPECT_TRUE(policy.should_retry(4, "error"));
    EXPECT_FALSE(policy.should_retry(5, "error"));
}

TEST(ExponentialBackoffPolicyTest, ExponentialDelay) {
    ExponentialBackoffPolicy policy(10, std::chrono::milliseconds{100}, 2.0);

    EXPECT_EQ(policy.get_delay(0).count(), 100); // 100 * 2^0
    EXPECT_EQ(policy.get_delay(1).count(), 200); // 100 * 2^1
    EXPECT_EQ(policy.get_delay(2).count(), 400); // 100 * 2^2
    EXPECT_EQ(policy.get_delay(3).count(), 800); // 100 * 2^3
}

TEST(ExponentialBackoffPolicyTest, MaxDelayLimit) {
    ExponentialBackoffPolicy policy(10, std::chrono::milliseconds{100}, 2.0,
                                    std::chrono::milliseconds{500});

    EXPECT_EQ(policy.get_delay(0).count(), 100);
    EXPECT_EQ(policy.get_delay(1).count(), 200);
    EXPECT_EQ(policy.get_delay(2).count(), 400);
    EXPECT_EQ(policy.get_delay(3).count(), 500);  // Capped at max
    EXPECT_EQ(policy.get_delay(10).count(), 500); // Still capped
}

TEST(JitteredBackoffPolicyTest, RetriesUpToMax) {
    JitteredBackoffPolicy policy(5);

    EXPECT_TRUE(policy.should_retry(0, "error"));
    EXPECT_TRUE(policy.should_retry(4, "error"));
    EXPECT_FALSE(policy.should_retry(5, "error"));
}

TEST(JitteredBackoffPolicyTest, DelayWithinRange) {
    JitteredBackoffPolicy policy(10, std::chrono::milliseconds{100}, 2.0,
                                 std::chrono::milliseconds{30000}, 0.5);

    // With 50% jitter, delay should be between 50% and 150% of base
    for (int i = 0; i < 100; ++i) {
        auto delay = policy.get_delay(0);
        EXPECT_GE(delay.count(), 50);  // 100 * 0.5
        EXPECT_LE(delay.count(), 150); // 100 * 1.5
    }
}

TEST(ConditionalRetryPolicyTest, RetriesOnlyMatchingErrors) {
    auto conditional = ConditionalRetryPolicy::transient_errors(RetryPolicyFactory::fixed(3));

    // Should retry transient errors
    EXPECT_TRUE(conditional->should_retry(0, "Connection timeout"));
    EXPECT_TRUE(conditional->should_retry(0, "Temporary failure"));
    EXPECT_TRUE(conditional->should_retry(0, "Server busy"));

    // Should not retry permanent errors
    EXPECT_FALSE(conditional->should_retry(0, "Invalid input"));
    EXPECT_FALSE(conditional->should_retry(0, "Permission denied"));
    EXPECT_FALSE(conditional->should_retry(0, "Not found"));
}

TEST(ConditionalRetryPolicyTest, RespectsMaxRetries) {
    auto conditional = ConditionalRetryPolicy::transient_errors(RetryPolicyFactory::fixed(2));

    EXPECT_TRUE(conditional->should_retry(0, "timeout"));
    EXPECT_TRUE(conditional->should_retry(1, "timeout"));
    EXPECT_FALSE(conditional->should_retry(2, "timeout"));
}

TEST(RetryPolicyFactoryTest, CreatesPolicies) {
    auto no_retry = RetryPolicyFactory::no_retry();
    EXPECT_STREQ(no_retry->name(), "NoRetry");

    auto fixed = RetryPolicyFactory::fixed(3);
    EXPECT_STREQ(fixed->name(), "FixedRetry");

    auto exponential = RetryPolicyFactory::exponential(5);
    EXPECT_STREQ(exponential->name(), "ExponentialBackoff");

    auto jittered = RetryPolicyFactory::jittered(5);
    EXPECT_STREQ(jittered->name(), "JitteredBackoff");
}
