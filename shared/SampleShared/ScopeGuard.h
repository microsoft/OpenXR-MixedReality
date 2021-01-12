// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <atomic>

template <typename TFunction>
struct ScopeGuard {
    static_assert(std::is_invocable_v<TFunction>, "Type must be invocable function.");

    ScopeGuard(bool active, TFunction&& guard)
        : m_guard(std::move(guard))
        , m_active(active) {
    }

    ScopeGuard(ScopeGuard&&) = default;
    ScopeGuard& operator=(ScopeGuard&&) = default;

    ~ScopeGuard() {
        if (m_active.load()) {
            m_guard();
        }
    }

    void Activate() {
        m_active.store(true);
    }

    void Deactivate() {
        m_active.store(false);
    }

private:
    ScopeGuard(ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&) = delete;

    TFunction m_guard;
    std::atomic<bool> m_active;
};

// Usage: auto guard = MakeScopeGuard([&] { foobar; });
template <typename TFunction>
ScopeGuard<TFunction> MakeScopeGuard(TFunction&& guard) {
    return ScopeGuard<TFunction>(true, std::forward<TFunction>(guard));
}

// Usage: auto guard = MakeInactiveScopeGuard([&] { foobar; });
// if (some_condition == satisified) {
//      guard.Activate()
// }
template <typename TFunction>
ScopeGuard<TFunction> MakeInactiveScopeGuard(TFunction&& guard) {
    return ScopeGuard<TFunction>(false, std::forward<TFunction>(guard));
}

// Usage: auto guard = MakeFailureGuard([&] { foobar; });
// Executes guard only if destruction is caused due to exception unwind.
template <typename TFunction>
auto MakeFailureGuard(TFunction&& guard) {
    auto failureGuard = [initialUncaughtExceptionCount{std::uncaught_exceptions()}, guard{std::move(guard)}]() {
        // Execute only if this lambda is being destroyed due to exception stack unwind.
        // In the case of this guard running while an exception is being handled as part of a stack unwind due to another
        // exception, there may already be an uncaught exception that should ignored. This is why the exception count
        // is compared against a snapshot.
        if (std::uncaught_exceptions() > initialUncaughtExceptionCount) {
            guard();
        }
    };
    return MakeScopeGuard(std::move(failureGuard));
}
