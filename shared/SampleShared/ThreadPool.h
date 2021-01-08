// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <deque>

namespace sample {
    class ThreadPool final {
        // Move-only alternative to using std::function<void()>
        class UniqueFunction {
            struct TypelessFunction {
                virtual void Call() = 0;
                virtual ~TypelessFunction() {
                }
            };
            std::unique_ptr<TypelessFunction> m_impl;
            template <typename F>
            struct TypedFunction : TypelessFunction {
                F m_func;
                TypedFunction(F&& func)
                    : m_func(std::move(func)) {
                }
                void Call() override {
                    m_func();
                }
            };

        public:
            template <typename F>
            explicit UniqueFunction(F&& f)
                : m_impl(new TypedFunction<F>(std::move(f))) {
            }
            UniqueFunction() = delete;
            UniqueFunction(const UniqueFunction&) = delete;
            UniqueFunction& operator=(const UniqueFunction&) = delete;
            UniqueFunction(UniqueFunction&& other) noexcept = default;
            UniqueFunction& operator=(UniqueFunction&& other) noexcept = default;

            void operator()() {
                m_impl->Call();
            }
        };

        // The state shared between all of the threads in the thread pool.
        // This is what makes it possible for the thread pool to be destroyed by one of its own threads.
        struct SharedState : std::enable_shared_from_this<SharedState> {
            explicit SharedState(size_t threadCount) {
                m_threads.reserve(threadCount);
            }

            template<typename F>
            _Requires_lock_not_held_(m_mutex) bool SubmitUnique(F&& f) {
                {
                    std::lock_guard guard(m_mutex);
                    if (!m_allowSubmit) {
                        return false;
                    }
                    m_tasks.emplace_back(std::move(f));
                }
                m_cond.notify_one();
                return true;
            }

            _Requires_lock_not_held_(m_mutex) void AddThread() {
                std::lock_guard guard(m_mutex);
                m_threads.emplace_back([this]() {
                    if (auto keepAlive = shared_from_this()) {
                        for (;;) {
                            std::unique_lock lk(m_mutex);
                            m_cond.wait(lk, [this]() { return m_stopped || !m_tasks.empty(); });
                            // Don't stop until the queue is empty
                            if (!m_tasks.empty()) {
                                auto task = std::move(m_tasks.front());
                                m_tasks.pop_front();
                                lk.unlock();
                                task();
                            } else if (m_stopped) {
                                break;
                            }
                        }
                    }
                });
            }

            _Requires_lock_not_held_(m_mutex) void DisallowSubmit() {
                std::lock_guard guard(m_mutex);
                m_allowSubmit = false;
            }

            _Requires_lock_not_held_(m_mutex) void JoinAllThreads() {
                {
                    std::lock_guard guard(m_mutex);
                    m_stopped = true;
                }
                m_cond.notify_all();

                for (;;) {
                    std::unique_lock lk(m_mutex);
                    if (m_threads.empty()) {
                        break;
                    }
                    auto thread = std::move(m_threads.back());
                    m_threads.pop_back();
                    lk.unlock();
                    if (std::this_thread::get_id() == thread.get_id()) {
                        thread.detach();
                    } else if (thread.joinable()) {
                        thread.join();
                    }
                }
            }

        private:
            std::vector<std::thread> m_threads;
            std::deque<UniqueFunction> m_tasks;
            std::condition_variable m_cond;
            std::mutex m_mutex;
            bool m_allowSubmit{true};
            bool m_stopped{false};
        };

    public:
        // A default constructed ThreadPool does not have a shared state.
        // Most methods will throw an exception if called with a default constructed ThreadPool.
        ThreadPool() noexcept = default;

        explicit ThreadPool(size_t threadCount)
            : m_state{std::make_shared<SharedState>(threadCount)} {
            if (threadCount == 0) {
                throw std::invalid_argument("threadCount must be greater than zero");
            }
            for (size_t i = 0; i < threadCount; ++i) {
                m_state->AddThread();
            }
        }

        // The destructor will wait for all tasks to complete.
        ~ThreadPool() {
            if (m_state) {
                m_state->DisallowSubmit();
                m_state->JoinAllThreads();
            }
        }

        ThreadPool(ThreadPool&& other) = default;
        ThreadPool& operator=(ThreadPool&& other) = default;
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        template <typename F>
        bool Submit(F func) {
            if (m_state == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            return m_state->SubmitUnique(std::move(func));
        }
        bool Submit(std::function<void()>) = delete;
        bool Submit(std::nullptr_t) = delete;

        // Stops the thread pool from accepting more tasks and then waits for all queued tasks to complete.
        void StopAndWait() {
            if (m_state == nullptr) {
                throw std::system_error(std::make_error_code(std::errc::operation_not_permitted));
            }
            m_state->DisallowSubmit();
            m_state->JoinAllThreads();
        }

        // Returns true if the ThreadPool has an associated shared state.
        // It does not indicate whether the thread pool has any running threads.
        explicit operator bool() const noexcept {
            return m_state != nullptr;
        }

    private:
        std::shared_ptr<SharedState> m_state;
    };
} // namespace sample
