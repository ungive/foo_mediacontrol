#pragma once

// Alternate lightweight implementation of pfc::event, with similar method sigantures but no support for multi-event-wait-for-any.
// It's a safe drop-in replacmeent for the regular event - code trying to use unsupported methods will fail to compile rather than behave incorrectly.

// Rationale:
// Mac/Linux multi-wait-capable pipe-backed event is relatively expensive, in terms of CPU, memory and file descriptors opened.
// On Windows, event_std still outperforms regular win32 event but the difference is mostly insignificant in real life use cases.

#include <mutex>
#include <condition_variable>
namespace pfc {
    class event_std {
    public:
        void set_state(bool v = true) {
            std::scoped_lock lock(m_mutex);
            if (m_state != v) {
                m_state = v;
                if (v) m_condition.notify_all();
            }
        }
        bool is_set() const { return m_state; }
        void wait() {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [this] { return this->m_state; });
        }
        bool wait_for(double timeout) {
            if (timeout < 0) { wait(); return true; }
            std::unique_lock lock(m_mutex);
            return m_condition.wait_for(lock, std::chrono::duration<double>(timeout), [this] { return this->m_state; });
        }
        void wait_and_clear() {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [this] { return this->m_state; });
            m_state = false;
        }
        bool wait_for_and_clear(double timeout) {
            if ( timeout < 0 ) {wait_and_clear(); return true;}
            std::unique_lock lock(m_mutex);
            bool rv = m_condition.wait_for(lock, std::chrono::duration<double>(timeout), [this] { return this->m_state; });
            if ( rv ) m_state = false;
            return rv;
        }
    private:
        volatile bool m_state = false;
        std::condition_variable m_condition;
        std::mutex m_mutex;
    };
}
