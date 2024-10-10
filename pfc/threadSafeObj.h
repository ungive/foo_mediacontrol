#pragma once
namespace pfc {

    template<typename obj_t_, typename mutex_t_>
    struct threadSafeObjCommon {
        typedef mutex_t_ mutex_t;
        typedef obj_t_ obj_t;
        template<typename ... arg_t> threadSafeObjCommon( arg_t && ... arg ) : m_obj(std::forward<arg_t>(arg) ... ) {}
        mutex_t m_mutex;
        obj_t m_obj;
    };

    template<typename common_t>
    class threadSafeObjLock {
        typedef threadSafeObjLock<common_t> self_t;
        common_t & m_common;
    public:
        typename common_t::obj_t & operator*() { return m_common.m_obj; }
        typename common_t::obj_t * operator->() { return & m_common.m_obj; }
        
        threadSafeObjLock( common_t & arg ) : m_common(arg) { m_common.m_mutex.lock(); }
        ~threadSafeObjLock( ) { m_common.m_mutex.unlock(); }
        threadSafeObjLock( const self_t & ) = delete;
        void operator=( const self_t & ) = delete;
    };

    template<typename obj_t, typename mutex_t = pfc::mutex>
    class threadSafeObj {
        typedef threadSafeObjCommon<obj_t, mutex_t> common_t;
        typedef threadSafeObjLock<common_t> lock_t;
        common_t m_common;
    public:
        template<typename ... arg_t>
        threadSafeObj( arg_t && ... arg ) : m_common( std::forward<arg_t>(arg) ... ) {}
        
        lock_t get() { return lock_t(m_common); }
    };

}
