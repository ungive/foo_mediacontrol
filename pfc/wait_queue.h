#pragma once

#include <list>
#include "synchro.h"

namespace pfc {

	template<typename obj_t>
	class waitQueue {
	public:
		waitQueue() : m_eof() {}
		
		template<typename arg_t>
		void put( arg_t && obj ) {
			mutexScope guard( m_mutex );
			m_list.push_back( std::forward<arg_t>(obj) );
			if ( m_list.size() == 1 ) m_canRead.set_state( true );
		}

		void set_eof() {
			mutexScope guard(m_mutex);
			m_eof = true;
			m_canRead.set_state(true);
		}
        bool wait_read( double timeout ) {
            return m_canRead.wait_for( timeout );
        }
        eventHandle_t get_event_handle() {
            return m_canRead.get_handle();
        }

		bool get( obj_t & out ) {
			for ( ;; ) {
				m_canRead.wait_for(-1);
				mutexScope guard(m_mutex);
				auto i = m_list.begin();
				if ( i == m_list.end() ) {
					if (m_eof) return false;
					continue;
				}
				out = std::move(*i);
				m_list.erase( i );
				didGet();
				return true;
			}
		}

		bool get( obj_t & out, pfc::eventHandle_t hAbort, bool * didAbort = nullptr ) {
			if (didAbort != nullptr) * didAbort = false;
			for ( ;; ) {
				if (pfc::event::g_twoEventWait( hAbort, m_canRead.get_handle(), -1) == 1) {
					if (didAbort != nullptr) * didAbort = true;
					return false;
				}
				mutexScope guard(m_mutex);
				auto i = m_list.begin();
				if ( i == m_list.end() ) {
					if (m_eof) return false;
					continue;
				}
				out = std::move(*i);
				m_list.erase( i );
				didGet();
				return true;
			}
		}
		void clear() {
			mutexScope guard(m_mutex);
			m_eof = false;
			m_list.clear();
			m_canRead.set_state(false);
		}
	private:
		void didGet() {
			if (!m_eof && m_list.size() == 0) m_canRead.set_state(false);
		}
		bool m_eof;
		std::list<obj_t> m_list;
		mutex m_mutex;
		event m_canRead;
	};

	template<typename obj_t_>
	class waitQueue2 {
	protected:
		typedef obj_t_ obj_t;
		typedef std::list<obj_t_> list_t;
		virtual bool canWriteCheck(list_t const &) { return true; }

	public:
		void waitWrite() {
			m_canWrite.wait_for(-1);
		}
		bool waitWrite(pfc::eventHandle_t hAbort) {
			return event::g_twoEventWait( hAbort, m_canWrite.get_handle(), -1) == 2;
		}
		eventHandle_t waitWriteHandle() {
			return m_canWrite.get_handle();
		}

		waitQueue2() {
			m_canWrite.set_state(true);
		}

		template<typename arg_t>
		void put(arg_t && obj) {
			mutexScope guard(m_mutex);
			m_list.push_back(std::forward<arg_t>(obj));
			if (m_list.size() == 1) m_canRead.set_state(true);
			refreshCanWrite();
		}

		void set_eof() {
			mutexScope guard(m_mutex);
			m_eof = true;
			m_canRead.set_state(true);
			m_canWrite.set_state(false);
		}

		bool get(obj_t & out) {
			for (;; ) {
				m_canRead.wait_for(-1);
				mutexScope guard(m_mutex);
				auto i = m_list.begin();
				if (i == m_list.end()) {
					if (m_eof) return false;
					continue;
				}
				out = std::move(*i);
				m_list.erase(i);
				didGet();
				return true;
			}
		}

		typedef std::function<bool(obj_t&)> receive_peek_t;
		// Block until there's something to return + return multiple objects at once.
		// Use peek function (optional) to stop reading / leave remaining items for the next call to pick up.
		std::list<obj_t> receive(pfc::eventHandle_t hAbort, receive_peek_t peek = nullptr , bool* didAbort = nullptr) {
			if (didAbort != nullptr) *didAbort = false;
			std::list<obj_t> ret;
			for (bool retry = false; ; retry = true ) {
				// try without wait first, only place where this is really used can poll abort before/after without system calls
				if (retry && pfc::event::g_twoEventWait(hAbort, m_canRead.get_handle(), -1) == 1) {
					if (didAbort != nullptr) *didAbort = true;
					break;
				}
				mutexScope guard(m_mutex);
				auto i = m_list.begin();
				if (i == m_list.end()) {
					if (m_eof) break;
					continue;
				}
				bool bDidGet = false;
				do {
					if (peek && !peek(*i)) break;
					auto n = i; ++n;
					ret.splice(ret.end(), m_list, i);
					i = std::move(n);
					bDidGet = true;
				} while (i != m_list.end());
				if ( bDidGet ) didGet();
				break;
			}
			return ret;
		}

		bool get(obj_t & out, pfc::eventHandle_t hAbort, bool * didAbort = nullptr) {
			if (didAbort != nullptr) * didAbort = false;
			for (;; ) {
				if (pfc::event::g_twoEventWait(hAbort, m_canRead.get_handle(), -1) == 1) {
					if (didAbort != nullptr) * didAbort = true;
					return false;
				}
				mutexScope guard(m_mutex);
				auto i = m_list.begin();
				if (i == m_list.end()) {
					if (m_eof) return false;
					continue;
				}
				out = std::move(*i);
				m_list.erase(i);
				didGet();
				return true;
			}
		}

		void clear() {
			mutexScope guard(m_mutex);
			m_list.clear();
			m_eof = false;
			m_canRead.set_state(false);
			m_canWrite.set_state(true);
		}
	private:
		void didGet() {
            // mutex assumed locked
			if (m_eof) return;
			if (m_list.empty()) {
				m_canRead.set_state(false);
				m_canWrite.set_state(true);
			} else {
				m_canWrite.set_state(canWriteCheck(m_list));
			}
		}
		void refreshCanWrite() {
            // mutex assumed locked
			m_canWrite.set_state( !m_eof && canWriteCheck(m_list));
		}
		bool m_eof = false;
		std::list<obj_t> m_list;
		mutex m_mutex;
		event m_canRead, m_canWrite;
	};
}
