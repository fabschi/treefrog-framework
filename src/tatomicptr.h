#ifndef TATOMICPTR_H
#define TATOMICPTR_H

#include <atomic>
#include <TGlobal>

namespace Tf
{
    inline void threadFence()
    {
        atomic_thread_fence(std::memory_order_seq_cst);
    }
}


template <class T>
class TAtomicPtr
{
public:
    TAtomicPtr(T *value = nullptr);
    TAtomicPtr(const TAtomicPtr<T> &other);
    ~TAtomicPtr() {}

    operator T*() const;
    T *load(bool *mark = nullptr) const;
    void store(T *value);
    bool compareExchange(T *expected, T *newValue);
    bool compareExchangeStrong(T *expected, T *newValue);
    TAtomicPtr<T> &operator=(T *value);
    TAtomicPtr<T> &operator=(const TAtomicPtr<T> &other);

    void mark();
    void unmark();
    bool isMarked() const;

private:
    enum {
        MASK = 0x3
    };

    T *raw() const;
    std::atomic<T*> atomicPtr {nullptr};

    T_DISABLE_MOVE(TAtomicPtr)
};


template <class T>
inline TAtomicPtr<T>::TAtomicPtr(T *value)
    : atomicPtr(value)
{ }


template <class T>
inline TAtomicPtr<T>::TAtomicPtr(const TAtomicPtr<T> &other)
    : atomicPtr(other.atomicPtr.load())
{ }


template <class T>
inline TAtomicPtr<T>::operator T*() const
{
    return load();
}


template <class T>
inline T *TAtomicPtr<T>::load(bool *mark) const
{
    T *ptr = atomicPtr.load(std::memory_order_acquire);
    if (mark) {
        *mark = (quintptr)ptr & 0x1;
    }
    return (T*)((quintptr)ptr & ~MASK);
}


template <class T>
inline T *TAtomicPtr<T>::raw() const
{
    return atomicPtr.load(std::memory_order_acquire);
}


template <class T>
inline void TAtomicPtr<T>::store(T *value)
{
    atomicPtr.store(value, std::memory_order_release);
}


template <class T>
inline bool TAtomicPtr<T>::compareExchange(T *expected, T *newValue)
{
    return atomicPtr.compare_exchange_weak(expected, newValue);
}


template <class T>
inline bool TAtomicPtr<T>::compareExchangeStrong(T *expected, T *newValue)
{
    return atomicPtr.compare_exchange_strong(expected, newValue);
}


template <class T>
inline TAtomicPtr<T> &TAtomicPtr<T>::operator=(T *value)
{
    store(value);
    return *this;
}


template <class T>
inline TAtomicPtr<T> &TAtomicPtr<T>::operator=(const TAtomicPtr<T> &other)
{
    store(other.load());
    return *this;
}


template <class T>
inline void TAtomicPtr<T>::mark()
{
    for (;;) {
        T* ptr = load();
        if ((ptr & 0x1) || compareExchange(ptr, ptr | 0x1)) {
            break;
        }
    }
}


template <class T>
inline void TAtomicPtr<T>::unmark()
{
    for (;;) {
        T* ptr = load();
        if (!(ptr & 0x1) || compareExchange(ptr, ptr & ~0x1)) {
            break;
        }
    }
}


template <class T>
inline bool TAtomicPtr<T>::isMarked() const
{
    return load() & 0x1;
}

#endif // TATOMICPTR_H
