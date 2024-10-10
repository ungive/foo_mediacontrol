#pragma once

#include <functional>

#include <exception>

namespace ThreadUtils {
	class CRethrow {
	private:
		std::exception_ptr m_exception;
	public:
		bool exec( std::function<void () > f ) throw();
		void rethrow() const;
		bool didFail() const { return !!m_exception; }
		void clear() { m_exception = nullptr; }
	};
}
