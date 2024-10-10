#pragma once

namespace pfc {
#ifdef _WIN32
    typedef HANDLE fileHandle_t;
    const fileHandle_t fileHandleInvalid = INVALID_HANDLE_VALUE;
#else
    typedef int fileHandle_t;
    constexpr fileHandle_t fileHandleInvalid = -1;
#endif
    
    void fileHandleClose( fileHandle_t h ) noexcept;
    fileHandle_t fileHandleDup( fileHandle_t h );
    
    class fileHandle {
    public:
        fileHandle( fileHandle_t val ) : h(val) {}
        fileHandle() : h ( fileHandleInvalid ) {}
        ~fileHandle() noexcept { close(); }
        fileHandle( fileHandle && other ) noexcept { h = other.h; other.clear(); }
        void operator=( fileHandle && other ) noexcept { close(); h = other.h; other.clear(); }
        void operator=( fileHandle_t other ) { close(); h = other; }
        void close() noexcept;
        void clear() noexcept { h = fileHandleInvalid; }
        bool isValid() noexcept { return h != fileHandleInvalid; }
        fileHandle_t h;
    private:
        fileHandle( const fileHandle & ) = delete;
        void operator=( const fileHandle & ) = delete;
    };
}
