#pragma once

#include "hts/types.hpp"
#include <chrono>
#include <functional>
#include <random>
#include <cmath>

namespace hts {

/// RetryPolicy defines how failed tasks should be retried
class RetryPolicy {
public:
    virtual ~RetryPolicy() = default;
    
    /// Check if task should be retried
    /// @param attempt Current attempt number (0-based)
    /// @param error Error message from last attempt
    /// @return true if should retry
    virtual bool should_retry(size_t attempt, const std::string& error) const = 0;
    
    /// Get delay before next retry
    /// @param attempt Current attempt number (0-based)
    /// @return Delay duration
    virtual std::chrono::milliseconds get_delay(size_t attempt) const = 0;
    
    /// Get policy name
    virtual const char* name() const = 0;
};

/// No retry policy - tasks are not retried
class NoRetryPolicy : public RetryPolicy {
public:
    bool should_retry(size_t, const std::string&) const override {
        return false;
    }
    
    std::chrono::milliseconds get_delay(size_t) const override {
        return std::chrono::milliseconds{0};
    }
    
    const char* name() const override { return "NoRetry"; }
};

/// Fixed retry policy - retry with constant delay
class FixedRetryPolicy : public RetryPolicy {
public:
    explicit FixedRetryPolicy(size_t max_retries = 3, 
                              std::chrono::milliseconds delay = std::chrono::milliseconds{100})
        : max_retries_(max_retries)
        , delay_(delay) {}
    
    bool should_retry(size_t attempt, const std::string&) const override {
        return attempt < max_retries_;
    }
    
    std::chrono::milliseconds get_delay(size_t) const override {
        return delay_;
    }
    
    const char* name() const override { return "FixedRetry"; }

private:
    size_t max_retries_;
    std::chrono::milliseconds delay_;
};

/// Exponential backoff retry policy
class ExponentialBackoffPolicy : public RetryPolicy {
public:
    explicit ExponentialBackoffPolicy(
        size_t max_retries = 5,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{100},
        double multiplier = 2.0,
        std::chrono::milliseconds max_delay = std::chrono::milliseconds{30000})
        : max_retries_(max_retries)
        , initial_delay_(initial_delay)
        , multiplier_(multiplier)
        , max_delay_(max_delay) {}
    
    bool should_retry(size_t attempt, const std::string&) const override {
        return attempt < max_retries_;
    }
    
    std::chrono::milliseconds get_delay(size_t attempt) const override {
        auto delay_ms = static_cast<long long>(
            initial_delay_.count() * std::pow(multiplier_, attempt));
        return std::chrono::milliseconds{
            std::min(delay_ms, max_delay_.count())
        };
    }
    
    const char* name() const override { return "ExponentialBackoff"; }

private:
    size_t max_retries_;
    std::chrono::milliseconds initial_delay_;
    double multiplier_;
    std::chrono::milliseconds max_delay_;
};

/// Exponential backoff with jitter (randomization)
class JitteredBackoffPolicy : public RetryPolicy {
public:
    explicit JitteredBackoffPolicy(
        size_t max_retries = 5,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{100},
        double multiplier = 2.0,
        std::chrono::milliseconds max_delay = std::chrono::milliseconds{30000},
        double jitter_factor = 0.5)
        : max_retries_(max_retries)
        , initial_delay_(initial_delay)
        , multiplier_(multiplier)
        , max_delay_(max_delay)
        , jitter_factor_(jitter_factor)
        , rng_(std::random_device{}()) {}
    
    bool should_retry(size_t attempt, const std::string&) const override {
        return attempt < max_retries_;
    }
    
    std::chrono::milliseconds get_delay(size_t attempt) const override {
        auto base_delay = static_cast<double>(
            initial_delay_.count() * std::pow(multiplier_, attempt));
        base_delay = std::min(base_delay, static_cast<double>(max_delay_.count()));
        
        // Add jitter: delay * (1 - jitter_factor + random * 2 * jitter_factor)
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        double jitter = 1.0 - jitter_factor_ + dist(rng_) * 2.0 * jitter_factor_;
        
        return std::chrono::milliseconds{
            static_cast<long long>(base_delay * jitter)
        };
    }
    
    const char* name() const override { return "JitteredBackoff"; }

private:
    size_t max_retries_;
    std::chrono::milliseconds initial_delay_;
    double multiplier_;
    std::chrono::milliseconds max_delay_;
    double jitter_factor_;
    mutable std::mt19937 rng_;
};

/// Conditional retry policy - retry based on error type
class ConditionalRetryPolicy : public RetryPolicy {
public:
    using ErrorPredicate = std::function<bool(const std::string&)>;
    
    explicit ConditionalRetryPolicy(
        std::unique_ptr<RetryPolicy> base_policy,
        ErrorPredicate predicate)
        : base_policy_(std::move(base_policy))
        , predicate_(std::move(predicate)) {}
    
    bool should_retry(size_t attempt, const std::string& error) const override {
        if (!predicate_(error)) {
            return false;  // Don't retry for non-retryable errors
        }
        return base_policy_->should_retry(attempt, error);
    }
    
    std::chrono::milliseconds get_delay(size_t attempt) const override {
        return base_policy_->get_delay(attempt);
    }
    
    const char* name() const override { return "ConditionalRetry"; }
    
    /// Create a policy that retries only transient errors
    static std::unique_ptr<ConditionalRetryPolicy> transient_errors(
        std::unique_ptr<RetryPolicy> base_policy) {
        return std::make_unique<ConditionalRetryPolicy>(
            std::move(base_policy),
            [](const std::string& error) {
                // Retry transient errors (timeout, temporary failure, etc.)
                return error.find("timeout") != std::string::npos ||
                       error.find("temporary") != std::string::npos ||
                       error.find("transient") != std::string::npos ||
                       error.find("retry") != std::string::npos ||
                       error.find("busy") != std::string::npos;
            });
    }

private:
    std::unique_ptr<RetryPolicy> base_policy_;
    ErrorPredicate predicate_;
};

/// Factory for creating common retry policies
class RetryPolicyFactory {
public:
    static std::unique_ptr<RetryPolicy> no_retry() {
        return std::make_unique<NoRetryPolicy>();
    }
    
    static std::unique_ptr<RetryPolicy> fixed(
        size_t max_retries = 3,
        std::chrono::milliseconds delay = std::chrono::milliseconds{100}) {
        return std::make_unique<FixedRetryPolicy>(max_retries, delay);
    }
    
    static std::unique_ptr<RetryPolicy> exponential(
        size_t max_retries = 5,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{100}) {
        return std::make_unique<ExponentialBackoffPolicy>(max_retries, initial_delay);
    }
    
    static std::unique_ptr<RetryPolicy> jittered(
        size_t max_retries = 5,
        std::chrono::milliseconds initial_delay = std::chrono::milliseconds{100}) {
        return std::make_unique<JitteredBackoffPolicy>(max_retries, initial_delay);
    }
};

} // namespace hts
