#pragma once

#include <memory>
#include <utility>
#include <type_traits>
#include "../Common.h"
#if __cplusplus >= 201703L
#include <shared_mutex>
#else
#include "RWSpinLock.hpp"
#endif

/**
 * \defgroup MemorySafety Memory Safety
 * \brief Helper classes, typedefs and functions for memroy safety.
 * \details
 *   Provide helperful functionalities for memory safety with:
 *     - Memory::RWSpinLock : A extremely high-performance read-write-spinlock
 *                            imported from folly library.\n
 *     - Memory::SafeSharedPtr : A wrapper to `std::shared_ptr` to provide
 *                               thread-safety while operating the underlying
 *                               pointer.\n
 *     - Memory::SafeWeakPtr : A wrapper to `std::weak_ptr` to cooperate with
 *                             Memory::SafeSharedPtr.
 * @{
 */

UTILITIES_NAMESPACE_BEGIN

/**
 * \namespace Memory
 * \brief Namespace for all classes, typedefs and functions of memory safety.
 *        See \ref MemorySafety for more instrucion.
 * @{
 */
namespace Memory {
// Forward declaration
template<typename T> class SafeWeakPtr;
template<typename T> class EnableSafeSharedFromThis;

/**
 * \brief Wrapper to `std::shared_ptr` to provide thread-safety while operating
 *        the underlying pointer.
 * \tparam T type of the object managed by SafeSharedPtr.
 * \details
 *   Same API as `std::shared_ptr`, but operator* and operator() are guarded by
 *   read-write lock to guarantee thread-safety.\n
 *   When `SafeSharedPtr` is a constant object, operations with underlying
 *   pointer will be guarded by read lock.\n
 *   When `SafeSharedPtr` is a mutable object, operations with underlying pointer
 *   will be guarded by write lock.\n
 *   \n
 *   **Sample Code**\n
 *   ```cpp
 *   Memory::SafeSharedPtr<int> ptr = Memory::make_shared<int>(0);
 *   std::thread thread([](Memory::SafeSharedPtr<int> ptr){
 *       for (int i = 0; i < 1000 * 1000; ++i)
 *           *ptr += 1;
 *   }, ptr);
 *   for (int i = 0; i < 1000 * 1000; ++i)
 *       *ptr += 1;
 *   thread.join();
 *   std::cout << *ptr << std::endl; // 2000000
 *   ```
 *   To prevent lock/unlock too often, call lock_shared() / lock(), then
 *   use get() for raw pointer directly, call unlock_shared() / unlock() when
 *   finished.\n
 *  \n
 *   See https://en.cppreference.com/w/cpp/memory/shared_ptr for more details of
 *   functionalities with std::shared_ptr.\n
 *
 *   -------------------------------------------------------------------
 *
 *   **Benchmark on (Intel(R) Core(R) CPU i5-6300U @ 2.4GHz Turbo 2.9GHz) 2 cores(4 HTs)**
 *
 *   Contention Benchmark on integer plus, 90% read, 10% write.
 *
 *   <table>
 *    <tr>
 *     <th>Data Type</th><th>Threads</th><th>Iters</th>
 *     <th>Total t</th><th>t/iter</th><th>iter/sec</th>
 *    </tr>
 *    <tr>
 *     <th colspan="6">MinGW 4.8.2 32bit on C++11 with RWSpinLock</th>
 *    </tr>
 *    <tr>
 *     <td>int*</td><td>1</td><td>1,000,000</td>
 *     <td>1.59 ms</td><td>1.59 ns</td><td>627 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>1</td><td>1,000,000</td>
 *     <td>17.62 ms</td><td>17.62 ns</td><td>56.73 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>4</td><td>1,000,000</td>
 *     <td>251.4 ms</td><td>62.85 ns</td><td>15.91 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>8</td><td>1,000,000</td>
 *     <td>720.57 ms</td><td>90.07 ns</td><td>11.1 M</td>
 *    </tr>
 *    <tr>
 *     <th colspan="6">MSVC 2017 64bit (cl 19.16.27024.1) on C++17 with std::shared_mutex</th>
 *    </tr>
 *    <tr>
 *     <td>int*</td><td>1</td><td>1,000,000</td>
 *     <td>1.61 ms</td><td>1.6 ns</td><td>620.96 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>1</td><td>1,000,000</td>
 *     <td>17.08 ms</td><td>17.08 ns</td><td>58.57 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>4</td><td>1,000,000</td>
 *     <td>80.81 ms</td><td>20.2 ns</td><td>49.5 M</td>
 *    </tr>
 *    <tr>
 *     <td>SafeSharedPtr</td><td>8</td><td>1,000,000</td>
 *     <td>159.16 ms</td><td>19.9 ns</td><td>50.26 M</td>
 *    </tr>
 *   </table>
 * \note
 *   **Before C++17**, for the purposes of the description below, a pointer type
 *   `Y*` requires that `Y*` must be implicitly convertible to `T*`.\n
 *   **Since C++17**, for the purposes of the description below, a pointer type
 *   `Y*` is said to be **compatible** with a pointer type `T*` if either `Y*` is
 *   convertible to `T*` or `Y` is the array type `U[N]` and `T` is `U cv []` (where
 *   cv is some set of cv-qualifiers).\n
 *   **Since C++17**, default deleter called on destructor will use `delete[]` if
 *   `T` is an arry type;
 * \warning
 *   Read-write lock used in this class is **NOT** recursive, so **DO NOT** call
 *   `operator.` `operator->` or `operator[]` multiply times in a single
 *   line/expression, otherwise a **deadlock** will happen. Sorry for the
 *   inconvenience.
 *   ```cpp
 *   // deadlock happens
 *   std::cout << point->x() << " " << point->y() << std::endl;
 *   ```
 *
 * \sa SafeWeakPtr, EnableSafeSharedFromThis
 */
template<typename T>
class SafeSharedPtr
{
public:
    class PtrHelper;
    class RefHelper;
    class ArrayHelper;

#if __cplusplus >= 201703L
    /**
     * \brief Defined to `std::shared_mutex` with C++17 or higher, otherwise
     *        defined to RWSpinLock.
     */
    using ReadWriteLock = std::shared_mutex;
    /**
     * \brief Defined to `std::shared_lock<std::shared_mutex>` with C++17 or
     *        higher, otherwise defined to RWSpinLock::ReadHolder.
     */
    using SharedLock = std::shared_lock<std::shared_mutex>;
    /**
     * \brief Defined to `std::unique_lock<std::shared_mutex>` with C++17 or
     *        higher, otherwise defined to RWSpinLock::WriteHolder.
     */
    using UniqueLock = std::unique_lock<std::shared_mutex>;
    /**
     * \brief Type of element managed. `T` for C++11, and
     *        `std::remove_extent_t<T>` for C++17.
     */
    using element_type = std::remove_extent_t<T>;
#else
    /**
     * \brief Defined to `std::shared_mutex` with C++17 or higher, otherwise
     *        defined to RWSpinLock.
     */
    using ReadWriteLock = RWSpinLock;
    /**
     * \brief Defined to `std::shared_lock<std::shared_mutex>` with C++17 or
     *        higher, otherwise defined to RWSpinLock::ReadHolder.
     */
    using SharedLock = RWSpinLock::ReadHolder;
    /**
     * \brief Defined to `std::unique_lock<std::shared_mutex>` with C++17 or
     *        higher, otherwise defined to RWSpinLock::WriteHolder.
     */
    using UniqueLock = RWSpinLock::WriteHolder;
    /**
     * \brief Type of element managed. `T` for C++11, and
     *        `std::remove_extent_t<T>` for C++17.
     */
    using element_type = T;
#endif
    /** \brief Type of weak pointer from this shared pointer. */
    using weak_type = SafeWeakPtr<T>;

    /**
     * \brief Default constructor, construct a `SafeSharedPtr` with no managed
     *        object, i.e. empty SafeSharedPtr.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     */
    constexpr SafeSharedPtr()
        : lck(std::make_shared<ReadWriteLock>())
    {}

    /**
     * \brief Construct a `SafeSharedPtr` with no managed object, i.e. empty
     *        SafeSharedPtr.
     * \param p nullptr.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     */
    constexpr SafeSharedPtr(std::nullptr_t p)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p)
    {}

    /**
     * \brief Constructs a `SafeSharedPtr` with a managed object.
     * \tparam  Y Type of input pointer.
     * \param   p Pointer to an object to manage.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete p` (if T is
     *   not an array type, `delete[] p` otherwise) (since C++17) is called if an
     *   exception occurs.
     */
    template<typename Y>
    explicit SafeSharedPtr(Y* p,
                           typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p)
    {
    }

    /**
     * \brief Constructs a `SafeSharedPtr` with a managed object.
     * \tparam  Y Type of input pointer.
     * \param   p Pointer to an object to manage.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete p` (if T is
     *   not an array type, `delete[] p` otherwise) (since C++17) is called if an
     *   exception occurs.
     */
    template<typename Y>
    explicit SafeSharedPtr(Y* p,
                           typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : ptr(p)
    { lck = ptr->__safeSharedLock; }

    /**
     * \brief Constructs a `SafeSharedPtr` with a managed object of specified
     *        deleter.
     * \tparam  Y       Type of input pointer.
     * \tparam  Deleter Type of specified deleter.
     * \param   p       Pointer to an object to manage.
     * \param   d       Deleter to use to destroy the object, must be
     *                  `CopyConstructible`, and expression d(ptr) must be well
     *                  formed, have well-defined behavior and not throw any
     *                  exceptions.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. d(p) is called if
     *   an exception occurs.
     */
    template<typename Y, typename Deleter>
    SafeSharedPtr(Y* p, Deleter d,
                  typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p, d)
    {
    }

    /**
     * \brief Constructs a `SafeSharedPtr` with a managed object of specified
     *        deleter.
     * \tparam  Y       Type of input pointer.
     * \tparam  Deleter Type of specified deleter.
     * \param   p       Pointer to an object to manage.
     * \param   d       Deleter to use to destroy the object, must be
     *                  `CopyConstructible`, and expression d(ptr) must be well
     *                  formed, have well-defined behavior and not throw any
     *                  exceptions.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. d(p) is called if
     *   an exception occurs.
     */
    template<typename Y, typename Deleter>
    SafeSharedPtr(Y* p, Deleter d,
                  typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : ptr(p, d)
    { lck = ptr->__safeSharedLock; }

    /**
     * \brief Constructs a `SafeSharedPtr` with with no managed but has specified
     *        deleter.
     * \tparam  Deleter Type of specified deleter.
     * \param   p       nullptr.
     * \param   d       Deleter to use to destroy the object, must be
     *                  `CopyConstructible`, and expression d(ptr) must be well
     *                  formed, have well-defined behavior and not throw any
     *                  exceptions.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. d(p) is called if
     *   an exception occurs.
     */
    template<typename Deleter>
    SafeSharedPtr(std::nullptr_t p, Deleter d)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p, d)
    {}

    /**
     * \brief Constructs a `SafeSharedPtr` with a managed object of specified
     *        deleter and allocator.
     * \tparam  Y       Type of input pointer.
     * \tparam  Deleter Type of specified deleter.
     * \tparam  Alloc   Type of specified allocator.
     * \param   p       Pointer to an object to manage.
     * \param   d       Deleter to use to destroy the object, must be
     *                  `CopyConstructible`, and expression d(ptr) must be well
     *                  formed, have well-defined behavior and not throw any
     *                  exceptions.
     * \param   alloc   Allocator to use for allocations of data for internal
     *                  use, must satisfy C++ named requirements of `Allocator`.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `d(p)` is called if
     *   an exception occurs.
     */
    template<typename Y, typename Deleter, typename Alloc>
    SafeSharedPtr(Y* p, Deleter d, Alloc alloc,
                  typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p, d, alloc)
    {}
    template<typename Y, typename Deleter, typename Alloc>
    SafeSharedPtr(Y* p, Deleter d, Alloc alloc,
                  typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : ptr(p, d, alloc)
    { lck = ptr->__safeSharedLock; }

    /**
     * \brief Constructs a `SafeSharedPtr` with no managed but has specified
     *        deleter and allocator.
     * \tparam  Deleter Type of specified deleter.
     * \tparam  Alloc   Type of specified allocator.
     * \param   p       nullptr.
     * \param   d       Deleter to use to destroy the object, must be
     *                  `CopyConstructible`, and expression d(ptr) must be well
     *                  formed, have well-defined behavior and not throw any
     *                  exceptions.
     * \param   alloc   Allocator to use for allocations of data for internal
     *                  use, must satisfy C++ named requirements of `Allocator`.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. d(p) is called if
     *   an exception occurs.
     */
    template<typename Deleter, typename Alloc>
    SafeSharedPtr(std::nullptr_t p, Deleter d, Alloc alloc)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(p, d, alloc)
    {}

    /**
     * \brief The aliasing constructor: constructs a `SafeSharedPtr` which shares
     *        ownership information with the initial value of `other`, but holds
     *        an unrelated and unmanaged pointer `p`.
     * \tparam  Y       Type of input shared pointer.
       \tparam  U       Type of input object, U must implicitly convertible to T.
     * \param   other   Input `SafeSharedPtr` to share ownership from.
     * \param   p       Pointer to an object to manage.
     * \details
     *   If this `SafeSharedPtr` is the last of the group to go out of scope, it
     *   will call the stored deleter for the object originally managed by
     *   other. However, calling get() on this `SafeSharedPtr` will always return
     *   a copy of `p`. It is the responsibility of the programmer to make sure
     *   that this pointer remains valid as long as this `SafeSharedPtr` exists,
     *   such as in the typical use cases where 'p' is a member of the object
     *   managed by `other` or is an alias (e.g., downcast) of `other.get()` after
     *   the call.
     */
    template<typename Y, typename U>
    SafeSharedPtr(const SafeSharedPtr<Y>& other, U* p) noexcept
        : lck(other.lck), ptr(other.ptr, p)
    {}

    /**
     * \brief The aliasing constructor: constructs a `SafeSharedPtr` which shares
     *        ownership information with the initial value of `other`, but holds
     *        an unrelated and unmanaged pointer `p`.
     * \tparam  Y       Type of input shared pointer.
     * \param   other   Input `SafeSharedPtr` to share ownership from.
     * \param   p       Pointer to an object to manage.
     * \details
     *   If this `SafeSharedPtr` is the last of the group to go out of scope, it
     *   will call the stored deleter for the object originally managed by
     *   other. However, calling get() on this `SafeSharedPtr` will always return
     *   a copy of `p`. It is the responsibility of the programmer to make sure
     *   that this pointer remains valid as long as this `SafeSharedPtr` exists,
     *   such as in the typical use cases where 'p' is a member of the object
     *   managed by `other` or is an alias (e.g., downcast) of `other.get()` after
     *   the call.
     */
    template<typename Y>
    SafeSharedPtr(const std::shared_ptr<Y>& other, T* p,
                  typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr) noexcept
        : lck(std::make_shared<ReadWriteLock>()), ptr(other, p)
    {}
    template<typename Y>
    SafeSharedPtr(const std::shared_ptr<Y>& other, T* p,
                  typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr) noexcept
        : lck(other->__safeSharedLock), ptr(other, p)
    {
    }

    /**
     * \brief Copy constructor, constructs a `SafeSharedPtr` which shares
     *        ownership of the object managed by `other`. If `other` manages no
     *        object, `*this` manages no object too.
     * \param other Another shared pointer to share the ownership from.
     */
    SafeSharedPtr(const SafeSharedPtr<T>& other) noexcept
        : lck(other.lck), ptr(other.ptr)
    {}

    /**
     * \brief Copy constructor, constructs a `SafeSharedPtr` which shares
     *        ownership of the object managed by `other`. If `other` manages no
     *        object, `*this` manages no object too.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to share the ownership from.
     */
    template<typename Y>
    SafeSharedPtr(const SafeSharedPtr<Y>& other) noexcept
        : lck(other.lck), ptr(other.ptr)
    {}

    /**
     * \brief Move constructor, move-constructs a `SafeSharedPtr` from `other`.
     *        After the construction, `*this` contains a copy of the previous
     *        state of `other`, `other` is empty and its stored pointer is null.
     * \param other Another shared pointer to acquire the ownership from.
     */
    SafeSharedPtr(SafeSharedPtr<T>&& other) noexcept
        : lck(std::forward<std::shared_ptr<ReadWriteLock>>(other.lck)),
          ptr(std::forward<std::shared_ptr<T>>(other.ptr))
    {}

    /**
     * \brief Move constructor, move-constructs a `SafeSharedPtr` from `other`.
     *        After the construction, `*this` contains a copy of the previous
     *        state of `other`, `other` is empty and its stored pointer is null.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to acquire the ownership from.
     */
    template<typename Y>
    SafeSharedPtr(SafeSharedPtr<Y>&& other) noexcept
        : lck(std::forward<std::shared_ptr<ReadWriteLock>>(other.lck)),
          ptr(std::forward<std::shared_ptr<Y>>(other.ptr))
    {}

    /**
     * \brief Constructs a `SafeSharedPtr` which shares ownership of the object
     *        managed by `other`. If `other` manages no object, `*this` manages no
     *        object too.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another weak pointer to share the ownership to.
     * \note `other.lock()` may be used for the same purpose: the difference is
     *       that this constructor throws an exception if the argument is empty,
     *       while `SafeWeakPtr<T>::lock()` constructs an empty SafeSharedPtr
     *       in that case.
     * \exception std::bad_weak_ptr
     *   Throw if `other.expired() == true` The constructor has no effect in
     *   this case.
     */
    template<typename Y>
    SafeSharedPtr(const SafeWeakPtr<Y>& other)
        : lck(other.lck), ptr(other.ptr)
    {}

    /**
     * \brief Constructs a `SafeSharedPtr` which shares ownership of the object
     *        managed by `other`, and provide read-write lock guard for memory
     *        safety. If `other` manages no object, `*this` manages no object too.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another `std::shared` pointer to share the ownership to.
     * \warning
     *   Only pointer operations with `SafeSharedPtr` are gauranteed memory safe,
     *   operations with existing `std::shared_ptr` are still without gaurantee.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     */
    template<typename Y>
    SafeSharedPtr(const std::shared_ptr<Y>& other,
                  typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : lck(std::make_shared<ReadWriteLock>()), ptr(other)
    {}
    template<typename Y>
    SafeSharedPtr(const std::shared_ptr<Y>& other,
                  typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : ptr(other)
    { lck = ptr->__safeSharedLock; }

    /**
     * \brief Move-constructs a `SafeSharedPtr` from `other`. After the
     *        construction, `*this` contains a copy of the previous state of
     *        `other`, `other` is empty and its stored pointer is null.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to acquire the ownership from.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     */
    template<typename Y>
    SafeSharedPtr(std::shared_ptr<Y>&& other,
                  typename std::enable_if<!std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : lck(std::make_shared<ReadWriteLock>()),
          ptr(std::forward<std::shared_ptr<Y>>(other))
    {}
    template<typename Y>
    SafeSharedPtr(std::shared_ptr<Y>&& other,
                  typename std::enable_if<std::is_base_of<EnableSafeSharedFromThis<Y>, Y>::value>::type* = nullptr)
        : ptr(std::forward<std::shared_ptr<Y>>(other))
    { lck = ptr->__safeSharedLock; }

    /**
     * \brief Constructs a `SafeSharedPtr` which shares ownership of the object
     *        managed by `other`. and provide read-write lock guard for memory
     *        safety. If `other` manages no object, `*this` manages no object too.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another weak pointer to share the ownership to.
     * \warning
     *   Only pointer operations with `SafeSharedPtr` are gauranteed memory safe,
     *   operations with existing `std::shared_ptr` are still without gaurantee.
     * \note `other.lock()` may be used for the same purpose: the difference is
     *       that this constructor throws an exception if the argument is empty,
     *       while `std::weak_ptr<T>::lock()` constructs an empty SafeSharedPtr
     *       in that case.
     * \exception std::bad_weak_ptr
     *   Throw if `other.expired() == true` The constructor has no effect in
     *   this case.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     */
    template<typename Y>
    SafeSharedPtr(const std::weak_ptr<Y>& other)
        : SafeSharedPtr(other.lock())
    {}

    /**
     * \brief Destructor, destructs the owned object if no more SafeSharedPtr
     *        link to it.
     * \details
     *   If `*this` owns an object and it is the last `SafeSharedPtr` owning it,
     *   the object is destroyed through the owned deleter. After the
     *   destruction, the shared pointers that shared ownership with `*this`, if
     *   any, will report a use_count() that is one less than its previous
     *   value.
     */
    ~SafeSharedPtr() = default;

    /**
     * \brief Shares ownership of the object managed by `other`.
     * \param other Another shared pointer to share the ownership to.
     * \return `*this` with same object managed by `other`.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(other).swap(*this)`.\n
     *   If `other` manages no object, `*this` manages no object too.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \sa reset
     */
    SafeSharedPtr<T>& operator=(const SafeSharedPtr<T>& other) noexcept
    {
        SafeSharedPtr<T>(other).swap(*this);
        return *this;
    }

    /**
     * \brief Move-assigns a `SafeSharedPtr` from `other`.
     * \param other Another shared pointer to acquire the ownership from.
     * \return `*this` with same object managed by `other`.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(std::move(other)).swap(*this)`.\n
     *   After the assignment, `*this` contains a copy of the previous state of
     *   `other`, and `other` is empty.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \sa reset
     */
    SafeSharedPtr<T>& operator=(SafeSharedPtr<T>&& other) noexcept
    {
        SafeSharedPtr<T>(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * \brief Shares ownership of the object managed by `other`.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to share the ownership to.
     * \return `*this` with same object managed by `other`.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(other).swap(*this)`.\n
     *   If `other` manages no object, `*this` manages no object too.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \sa reset
     */
    template<typename Y>
    SafeSharedPtr<T>& operator=(const SafeSharedPtr<Y>& other) noexcept
    {
        SafeSharedPtr<T>(other).swap(*this);
        return *this;
    }

    /**
     * \brief Move-assigns a `SafeSharedPtr` from `other`.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to acquire the ownership from.
     * \return `*this` with same object managed by `other`.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(std::move(other)).swap(*this)`.\n
     *   After the assignment, `*this` contains a copy of the previous state of
     *   `other`, and `other` is empty.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \sa reset
     */
    template<typename Y>
    SafeSharedPtr<T>& operator=(SafeSharedPtr<Y>&& other) noexcept
    {
        SafeSharedPtr<T>(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * \brief Shares ownership of the object managed by `other`, and provide
     *        read-write lock guard for memory safety.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to share the ownership to.
     * \return `*this` with same object managed by `other`.
     * \details
     *   If `other` manages no object, `*this` manages no object too.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \warning
     *   Only pointer operations with `SafeSharedPtr` are gauranteed memory safe,
     *   operations with existing `std::shared_ptr` are still without gaurantee.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     * \sa reset
     */
    template<typename Y>
    SafeSharedPtr<T>& operator=(const std::shared_ptr<Y>& other)
    {
        SafeSharedPtr<T>(other).swap(*this);
        return *this;
    }

    /**
     * \brief Move-assigns a `SafeSharedPtr` from `other`, provide read-write lock
     *        guard for memory safety.
     * \tparam  Y       Type of input pointer.
     * \param   other   Another shared pointer to acquire the ownership from.
     * \return `*this` with same object managed by `other`.
     * \details
     *   After the assignment, `*this` contains a copy of the previous state of
     *   `other`, and `other` is empty.\n
     *   Replaces the managed object with the one managed by `other`. If *this
     *   already owns an object and it is the last `SafeSharedPtr` owning it, and
     *   `other` is not the same as `*this`, the object is destroyed through the
     *   owned deleter.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     * \sa reset
     */
    template<typename Y>
    SafeSharedPtr<T>& operator=(std::shared_ptr<Y>&& other)
    {
        SafeSharedPtr<T>(std::move(other)).swap(*this);
        return *this;
    }

    /**
     * \brief Releases the ownership of the managed object, if any. After the
     *        call, `*this` manages no object.
     * \details
     *   Equivalent to `SafeSharedPtr().swap(*this)`.\n
     *   If `*this` already owns an object and it is the last SafeSharedPtr
     *   owning it, the object is destroyed through the owned deleter.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.
     * \sa operator=
     */
    void reset()
    { SafeSharedPtr<T>().swap(*this); }

    /**
     * \brief Replaces the managed object with an object pointed to by ptr. Uses
     *        the delete expression as the deleter.
     * \tparam  Y       Type of input pointer. A valid delete expression must be
     *                  available, i.e. `delete ptr` must be well formed, have
     *                  well-defined behavior and not throw any exceptions.
     * \param   ptr     Pointer to an object to acquire ownership of.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(ptr).swap(*this)`.\n
     *   Replaces the managed object with an object pointed to by `ptr`. By
     *   default, delete expression is used as deleter. Proper delete expression
     *   corresponding to the supplied type is always selected, this is the
     *   reason why the function is implemented as template using a separate
     *   parameter `Y`.\n
     *   If `*this` already owns an object and it is the last SafeSharedPtr
     *   owning it, the object is destroyed through the owned deleter.\n
     *   If the object pointed to by `ptr` is already owned, the function results
     *   in undefined behavior.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete ptr` is
     *   called if an exception occurs.
     * \sa operator=
     */
    template<typename Y>
    void reset(Y* ptr)
    { SafeSharedPtr<T>(ptr).swap(*this); }

    /**
     * \brief Replaces the managed object with an object pointed to by ptr. Uses
     *        the specified deleter `d` as the deleter.
     * \tparam  Y       Type of input pointer.
     * \tparam  Deleter Type of specified deleter. Deleter must be callable for
     *                  the type `T`, i.e. `d(ptr)` must be well formed, have
     *                  well-defined behavior and not throw any exceptions.
     *                  Deleter must be **CopyConstructible**, and its copy
     *                  constructor and destructor must not throw exceptions.
     * \param   ptr     Pointer to an object to acquire ownership of.
     * \param   d       Deleter to store for deletion of the object.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(ptr, d).swap(*this)`.\n
     *   Replaces the managed object with an object pointed to by `ptr`. Optional
     *   deleter `d` is supplied, which is later used to destroy the new object
     *   when no `SafeSharedPtr` objects own it.\n
     *   If `*this` already owns an object and it is the last SafeSharedPtr
     *   owning it, the object is destroyed through the owned deleter.\n
     *   If the object pointed to by `ptr` is already owned, the function results
     *   in undefined behavior.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `d(ptr)` is called if
     *   an exception occurs.
     * \sa operator=
     */
    template<typename Y, typename Deleter>
    void reset(Y* ptr, Deleter d)
    { SafeSharedPtr<T>(ptr, d).swap(*this); }

    /**
     * \brief Replaces the managed object with an object pointed to by ptr. Uses
     *        the specified deleter `d` as the deleter. Uses a copy of alloc for
     *        allocation of data for internal use.
     * \tparam  Y       Type of input pointer.
     * \tparam  Deleter Type of specified deleter. Deleter must be callable for
     *                  the type `T`, i.e. `d(ptr)` must be well formed, have
     *                  well-defined behavior and not throw any exceptions.
     *                  Deleter must be **CopyConstructible**, and its copy
     *                  constructor and destructor must not throw exceptions.
     * \tparam  Alloc   Type of specified allocator. Alloc must satisfy C++
     *                  named requirements of `Allocator`. The copy constructor
     *                  and destructor must not throw exceptions.
     * \param   ptr     Pointer to an object to acquire ownership of.
     * \param   d       Deleter to store for deletion of the object.
     * \param   alloc   Allocator to use for allocations of data for internal
     *                  use, must satisfy C++ named requirements of `Allocator`.
     * \details
     *   Equivalent to `SafeSharedPtr<T>(ptr, d, alloc).swap(*this)`.\n
     *   Replaces the managed object with an object pointed to by `ptr`. Optional
     *   deleter `d` is supplied, which is later used to destroy the new object
     *   when no `SafeSharedPtr` objects own it.\n
     *   If `*this` already owns an object and it is the last SafeSharedPtr
     *   owning it, the object is destroyed through the owned deleter.\n
     *   If the object pointed to by `ptr` is already owned, the function results
     *   in undefined behavior.
     * \exception std::bad_alloc
     *   If read-write lock could not be obtained. May throw
     *   implementation-defined exception for other errors. `delete lck` is
     *   called if an exception occurs.\n
     *   If required additional memory could not be obtained. May throw
     *   implementation-defined exception for other errors. `d(ptr)` is called if
     *   an exception occurs.
     * \sa operator=
     */
    template<typename Y, typename Deleter, typename Alloc>
    void reset(Y* ptr, Deleter d, Alloc alloc)
    { SafeSharedPtr<T>(ptr, d, alloc).swap(*this); }

    /**
     * \brief Exchanges the contents of `*this` and `other`.
     * \param other Another shared pointer to exchange the contents with.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    void swap(SafeSharedPtr<T>& other) noexcept
    {
        lck.swap(other.lck);
        ptr.swap(other.ptr);
    }

    /**
     * \brief Returns the stored pointer.
     * \return The stored pointer.
     * \note A `SafeSharedPtr` may share ownership of an object while storing a
     *       pointer to another object. `get()` returns the stored pointer, not
     *       the managed pointer.
     * \warning Thread safety is not gauranteed with this method, call
     *          lock_shared() / lock() before get(), and call
     *          unlock_shared() / unlock() when finished.
     * \sa operator*, operator->
     */
    element_type* get() const noexcept
    { return ptr.get(); }

    /**
     * \brief Dereferences the stored pointer, guard it with **write lock**. The
     *        behavior is undefined if the stored pointer is null.
     * \result A temporary object provides proxy to dereferencing the stored
     *         pointer, with lock() on construction and unlock() on
     *         destruction.
     * \note This method is thread-safe.
     * \sa get
     */
    RefHelper operator*() noexcept
    { return RefHelper(*this); }

    /**
     * \brief Dereferences the stored pointer, guard it with **read lock**. The
     *        behavior is undefined if the stored pointer is null.
     * \result A temporary object provides proxy to dereferencing the stored
     *         pointer, with lock_shared() on construction and unlock_shared() on
     *         destruction.
     * \note This method is thread-safe.
     * \sa get
     */
    const RefHelper operator*() const noexcept
    { return RefHelper(*this); }

    /**
     * \brief Dereferences the stored pointer, guard it with **write lock**. The
     *        behavior is undefined if the stored pointer is null.
     * \result A temporary object provides proxy to the stored pointer, with
     *         lock() on construction and unlock() on destruction.
     * \note This method is thread-safe.
     * \sa get
     */
    PtrHelper operator->() noexcept
    { return PtrHelper(*this); }

    /**
     * \brief Dereferences the stored pointer, guard it with **read lock**. The
     *        behavior is undefined if the stored pointer is null.
     * \result A temporary object provides proxy to the stored pointer, with
     *         lock_shared() on construction and unlock_shared() on destruction.
     * \note This method is thread-safe.
     * \sa get
     */
    const PtrHelper operator->() const noexcept
    { return PtrHelper(*this); }

#if __cplusplus >= 201703L
    /**
     * \brief Provides indexed access to the stored array, guard it with
     *        **write lock**.
     * \param idx The array index.
     * \result A temporary object provides proxy to the idx-th element of the
     *         array, with lock() on construction and unlock() on
     *         destruction.
     * \details
     *   The behavior is undefined if the stored pointer is null or if `idx` is
     *   negative.\n
     *   If `T` (the template parameter of SafeSharedPtr) is an array type `U[N]`,
     *   `idx` must be less than `N`, otherwise the behavior is undefined.\n
     *   When `T` is not an array type, it is unspecified whether this function
     *   is declared. If the function is declared, it is unspecified what its
     *   return type is, except that the declaration (although not necessarily
     *   the definition) of the function is guaranteed to be legal.
     * \note This method is thread-safe.
     * \sa get
     */
    ArrayHelper operator[](std::ptrdiff_t idx)
    { return ArrayHelper(*this, idx); }

    /**
     * \brief Provides indexed access to the stored array, guard it with
     *        **read lock**.
     * \param idx The array index.
     * \result A temporary object provides proxy to the idx-th element of the
     *         array, with lock_shared() on construction and unlock_shared() on
     *         destruction.
     * \details
     *   The behavior is undefined if the stored pointer is null or if `idx` is
     *   negative.\n
     *   If `T` (the template parameter of SafeSharedPtr) is an array type `U[N]`,
     *   `idx` must be less than `N`, otherwise the behavior is undefined.\n
     *   When `T` is not an array type, it is unspecified whether this function
     *   is declared. If the function is declared, it is unspecified what its
     *   return type is, except that the declaration (although not necessarily
     *   the definition) of the function is guaranteed to be legal.
     * \sa get
     */
    const ArrayHelper& operator[](std::ptrdiff_t idx) const
    { return ArrayHelper(*this, idx); }
#endif

    /**
     * \brief Returns the number of `SafeSharedPtr` objects referring to the same
     *        managed object.
     * \return The number of `SafeSharedPtr` instances managing the current object
     *         or 0 if there is no managed object.
     * \details
     *   Returns the number of different `SafeSharedPtr` instances (this included)
     *   managing the current object. If there is no managed object, 0 is
     *   returned.\n
     *   In multithreaded environment, the value returned by use_count is
     *   approximate (typical implementations use a `memory_order_relaxed` load)
     * \note
     *   Common use cases include:\n
     *     - Comparison with 0. If use_count returns zero, the shared pointer is
     *       empty and manages no objects (whether or not its stored pointer is
     *       null). In multithreaded environment, this does not imply that the
     *       destructor of the managed object has completed.
     *     - Comparison with 1. If use_count returns 1, there are no other
     *       owners. In multithreaded environment, this does not imply that the
     *       object is safe to modify because accesses to the managed object by
     *       former shared owners may not have completed, and because new shared
     *       owners may be introduced concurrently, such as by
     *       SafeWeakPtr::lock.
     */
    long use_count() const noexcept
    { return ptr.use_count(); }

    /**
     * \brief Checks if *this stores a non-null pointer, i.e. whether
     *        `get() != nullptr`.
     * \return `true` if `*this` stores a pointer, `false` otherwise.
     * \note An empty `SafeSharedPtr` (where `use_count() == 0`) may store a
     *       non-null pointer accessible by get(), e.g. if it were created using
     *       the aliasing constructor.
     * \sa get
     */
    explicit operator bool() const noexcept
    { return get() != nullptr; }

    /**
     * \brief Checks whether this `SafeSharedPtr` precedes other in implementation
     *        defined owner-based (as opposed to value-based) order.
     * \tparam  Y       The type of input operand.
     * \param   other   the `SafeSharedPtr` to be compared.
     * \return `true` if `*this` precedes other, `false` otherwise. Common
     *         implementations compare the addresses of the control blocks.
     * \details
     *   The order is such that two shared pointers compare equivalent only if
     *   they are both empty or if they both own the same object, even if the
     *   values of the pointers obtained by get() are different (e.g. because
     *   they point at different subobjects within the same object) \n
     *   This ordering is used to make shared and weak pointers usable as keys
     *   in associative containers, typically through std::owner_less.
     */
    template<typename Y>
    bool owner_before(const SafeSharedPtr<Y>& other) const
    { return ptr.owner_before(other.ptr); }

    /**
     * \brief Checks whether this `SafeSharedPtr` precedes other in implementation
     *        defined owner-based (as opposed to value-based) order.
     * \tparam  Y       The type of input operand.
     * \param   other   the SafeWeakPtr to be compared.
     * \return `true` if `*this` precedes other, `false` otherwise. Common
     *         implementations compare the addresses of the control blocks.
     * \details
     *   The order is such that two shared pointers compare equivalent only if
     *   they are both empty or if they both own the same object, even if the
     *   values of the pointers obtained by get() are different (e.g. because
     *   they point at different subobjects within the same object) \n
     *   This ordering is used to make shared and weak pointers usable as keys
     *   in associative containers, typically through std::owner_less.
     */
    template<typename Y>
    bool owner_before(const SafeWeakPtr<Y>& other) const
    { return ptr.owner_before(other.ptr); }

    /**
     * \brief Locks the lock for reading. This function will block the current
     *        thread if another thread has locked for writing.
     * \details
     *   Multiply readers in different thread can lock_shared at same time.\n
     *   It is not possible to lock for read if the thread already has locked
     *   for write.
     * \note This method is thread-safe.
     * \warning Read-write lock is `NOT` recursive, locking multiply times in
     *          same thread will cause block.
     * \sa unlock_shared
     */
    void lock_shared() const
    { lck->lock_shared(); }

    /**
     * \brief Unlocks the read lock.
     * \details
     *   Attempting to unlock a lock that is not locked is an error, and will
     *   result in undefined behaviour.
     * \note This method is thread-safe.
     * \sa lock_shared
     */
    void unlock_shared() const
    { lck->unlock_shared(); }

    /**
     * \brief Locks the lock for writing. This function will block the current
     *        thread if another thread (including the current) has locked for
     *        reading or writing.
     * \details
     *   Only `one` write can lock at same time.\n
     *   It is not possible to lock for read if the thread already has locked
     *   for write.
     * \note This method is thread-safe.
     * \warning Read-write lock is `NOT` recursive, locking multiply times in
     *          same thread will cause block.
     * \sa unlock
     */
    void lock()
    { lck->lock(); }

    /**
     * \brief Unlocks the write lock.
     * \details
     *   Attempting to unlock a lock that is not locked is an error, and will
     *   result in undefined behaviour.
     * \note This method is thread-safe.
     * \sa lock
     */
    void unlock() const
    { lck->unlock(); }

    /**
     * \brief Generate a RAII guard for read lock, it will call lock_shared() on
     *        construction and unlock_shared() on destruction.
     * \return RAII guard for read lock
     * \note This method is thread-safe.
     * \sa lock_shared, unlock_shared
     */
    SharedLock shared_lock() const
    { return SharedLock(*lck); }

    /**
     * \brief Generate a RAII guard for write lock, it will call lock()
     *        on construction and unlock() on destruction.
     * \return RAII write for read lock
     * \note This method is thread-safe.
     * \sa lock, unlock
     */
    UniqueLock unique_lock() const
    { return UniqueLock(*lck); }

    /**
     * \brief Proxy class for operator-> in SafeSharedPtr, behave like
     *        underlying object, and provide RAII read-write lock for
     *        thread safety.
     * \details
     *   If constructed as constant, it will call SafeSharedPtr::lock_shared()
     *   on construction and SafeSharedPtr::unlock_shared() on destruction.\n
     *   If constructed as mutable, it will call SafeSharedPtr::lock() on
     *   construction and SafeSharedPtr::unlock() on destruction.
     * \note
     *   Copy constructor and copy assignment are deleted to prevent multiply
     *   locks, use `std::move` with move constructor and move assignment to
     *   transport it's ownership, or simply use it like type T* or T&.
     * \sa SafeSharedPtr
     */
    class PtrHelper
    {
    public:
        /** \brief Pointer type of element. */
        using pointer = T*;
        /** \brief Const pointer type of element. */
        using const_pointer = const T*;
        /** \brief Reference type of element. */
        using reference = T&;
        /** \brief Const eference type of element. */
        using const_reference = const T&;

        /**
         * \brief Constructor a constant PtrHelper to gain access to underlying
         *        object of SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock_shared() on construction.
         * \param p `SafeSharedPtr` to access from.
         */
        explicit PtrHelper(SafeSharedPtr<T>& p)
            : ptr(&p)
        { ptr->lock(); }

        /**
         * \brief Constructor a mutable PtrHelper to gain access to underlying
         *        object of SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock() on construction.
         * \param p `SafeSharedPtr` to access from.
         */
        explicit PtrHelper(const SafeSharedPtr<T>& p)
            : constPtr(&p)
        { constPtr->lock_shared(); }

        /**
         * \brief Move constructor, transport ownership to another PtrHelper,
         *        keep existing lock state.
         * \param other Another PtrHelper to move to.
         */
        PtrHelper(PtrHelper&& other) noexcept
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
        }

        /**
         * \brief Destructor, call SafeSharedPtr::unlock_shared() if constructed
         *        as constant, otherwise call SafeSharedPtr::lock() if
         *        constructed as mutable.
         */
        ~PtrHelper()
        {
            if (ptr) ptr->unlock();
            if (constPtr) constPtr->unlock_shared();
        }

        /**
         * \brief Move assigment, transport ownership to another PtrHelper, keep
         *        existing lock state.
         * \param other Another PtrHelper to move to.
         * \return `*this` with empty content.
         */
        PtrHelper& operator=(PtrHelper&& other)
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
            return *this;
        }

        /**
         * \brief Operator overload to act as `T*`.
         * \return `T&`.
         */
        operator pointer()
        { return ptr->get(); }

        /**
         * \brief Operator overload to act as `const T*`.
         * \return `const T&`.
         */
        operator const_pointer() const
        { return constPtr->get(); }

        /**
         * \brief Operator overload to act as `T*`.
         * \return `T*`.
         */
        pointer operator->()
        { return ptr->get(); }

        /**
         * \brief Operator overload to act as `const T*`.
         * \return `const T*`.
         */
        const_pointer operator->() const
        {
            if (constPtr) return constPtr->get();
            else return ptr->get();
        }

    private:
        SafeSharedPtr<T>* ptr = nullptr;
        const SafeSharedPtr<T>* constPtr = nullptr;

        PtrHelper(const PtrHelper&) = delete;
        PtrHelper& operator=(const PtrHelper&) = delete;
    };

    /**
     * \brief Proxy class for operator* in SafeSharedPtr, behave like underlying
     *        object, and provide RAII read-write lock for
     *        thread safety.
     * \details
     *   If constructed as constant, it will call SafeSharedPtr::lock_shared()
     *   on construction and SafeSharedPtr::unlock_shared() on destruction.\n
     *   If constructed as mutable, it will call SafeSharedPtr::lock() on
     *   construction and SafeSharedPtr::unlock() on destruction.
     * \note
     *   Copy constructor and copy assignment are deleted to prevent multiply
     *   locks, use `std::move` with move constructor and move assignment to
     *   transport it's ownership, or simply use it like type T* or T&.\n
     *   Because operator. cannot be overloaded, `*pPoint.x` cannot compile, use
     *   `pPoint->x` instead. Sorry for that.
     * \sa SafeSharedPtr
     */
    class RefHelper
    {
    public:
        /** \brief Reference type of element. */
        using reference = T&;
        /** \brief Const eference type of element. */
        using const_reference = const T&;

        /**
         * \brief Constructor a constant RefHelper to gain access to underlying
         *        object of SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock_shared() on construction.
         * \param p `SafeSharedPtr` to access from.
         */
        explicit RefHelper(SafeSharedPtr<T>& p)
            : ptr(&p)
        { ptr->lock(); }

        /**
         * \brief Constructor a mutable RefHelper to gain access to underlying
         *        object of SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock() on construction.
         * \param p `SafeSharedPtr` to access from.
         */
        explicit RefHelper(const SafeSharedPtr<T>& p)
            : constPtr(&p)
        { constPtr->lock_shared(); }

        /**
         * \brief Move constructor, transport ownership to another RefHelper,
         *        keep existing lock state.
         * \param other Another RefHelper to move to.
         */
        RefHelper(RefHelper&& other) noexcept
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
        }

        /**
         * \brief Destructor, call SafeSharedPtr::unlock_shared() if constructed
         *        as constant, otherwise call SafeSharedPtr::lock() if
         *        constructed as mutable.
         */
        ~RefHelper()
        {
            if (ptr) ptr->unlock();
            if (constPtr) constPtr->unlock_shared();
        }

        /**
         * \brief Move assigment, transport ownership to another RefHelper, keep
         *        existing lock state.
         * \param other Another RefHelper to move to.
         * \return `*this` with empty content.
         */
        RefHelper& operator=(RefHelper&& other)
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
            return *this;
        }

        /**
         * \brief Operator overload to act as `T&`.
         * \return `T&`.
         */
        operator reference()
        { return *(ptr->get()); }

        /**
         * \brief Operator overload to act as `const T&`.
         * \return `const T&`.
         */
        operator const_reference() const
        {
            if (constPtr) return *(constPtr->get());
            else return *(ptr->get());
        }

        /**
         * \brief Assign operator to assign from another value.
         * \tparam  X       Type of input, `X&` must be implicitly convertible to
         *                  `T&`.
         * \param   other   Input value to be assigned from.
         * \details
         *   Used for situations like:
         *   ```cpp
         *   auto ptr = Memory::make_shared<int>(0);
         *   *ptr = 42;
         *   ```
         */
        template<typename X>
        RefHelper& operator=(const X& other)
        {
            operator reference() = other;
            return *this;
        }

    private:
        SafeSharedPtr<T>* ptr = nullptr;
        const SafeSharedPtr<T>* constPtr = nullptr;

        RefHelper(const RefHelper&) = delete;
        RefHelper& operator=(const RefHelper&) = delete;
    };

    #if __cplusplus >= 201703L
    /**
     * \brief Proxy class for operator[] in SafeSharedPtr, behave like array
     *        element of underlying array object, and provide RAII read-write
     *        lock for thread safety.
     * \details
     *   If constructed as constant, it will call SafeSharedPtr::lock_shared()
     *   on construction and SafeSharedPtr::unlock_shared() on destruction.\n
     *   If constructed as mutable, it will call SafeSharedPtr::lock() on
     *   construction and SafeSharedPtr::unlock() on destruction.
     * \note
     *   Copy constructor and copy assignment are deleted to prevent multiply
     *   locks, use `std::move` with move constructor and move assignment to
     *   transport it's ownership, or simply use it like type T* or T&.\n
     *   Because operator. cannot be overloaded, `pPoints[0].x` cannot compile,
     *   use `Point(pPoints[0]).x` instead. Sorry for that.
     * \warning
     *   Behavior is undefined if `T` is not array type.
         * \sa SafeSharedPtr
     */
    class ArrayHelper
    {
    public:
        /** \brief Element type of array `T`. */
        using element_type = std::remove_extent_t<T>;
        /** \brief Pointer type of element. */
        using pointer = element_type*;
        /** \brief Const pointer type of element. */
        using const_pointer = const element_type*;
        /** \brief Reference type of element. */
        using reference = element_type&;
        /** \brief Const eference type of element. */
        using const_reference = const element_type&;

        /**
         * \brief Constructor a constant ArrayHelper to gain access to element
         *        of object managed by SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock_shared() on construction.
         * \param p     `SafeSharedPtr` to access from.
         * \param idx   Index for element in array to access from.
         */
        ArrayHelper(SafeSharedPtr<T>& p, std::ptrdiff_t idx)
            : ptr(&p), index(idx)
        { ptr->lock(); }

        /**
         * \brief Constructor a mutable ArrayHelper to gain access to element
         *        of object managed by SafeSharedPtr.
         * \details
         *   Will call SafeSharedPtr::lock() on construction.
         * \param p     `SafeSharedPtr` to access from.
         * \param idx   Index for element in array to access from.
         */
        ArrayHelper(const SafeSharedPtr<T>& p, std::ptrdiff_t idx)
            : constPtr(&p), index(idx)
        { constPtr->lock_shared(); }

        /**
         * \brief Move constructor, transport ownership to another ArrayHelper,
         *        keep existing lock state.
         * \param other Another ArrayHelper to move to.
         */
        ArrayHelper(ArrayHelper&& other) noexcept
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
            std::swap(index, other.index);
        }

        /**
         * \brief Destructor, call SafeSharedPtr::unlock_shared() if constructed
         *        as constant, otherwise call SafeSharedPtr::lock() if
         *        constructed as mutable.
         */
        ~ArrayHelper()
        {
            if (ptr) ptr->unlock();
            if (constPtr) constPtr->unlock_shared();
        }

        /**
         * \brief Move assigment, transport ownership to another ArrayHelper,
         *        keep existing lock state.
         * \param other Another ArrayHelper to move to.
         * \return `*this` with empty content.
         */
        ArrayHelper& operator=(ArrayHelper&& other)
        {
            std::swap(ptr, other.ptr);
            std::swap(constPtr, other.constPtr);
            std::swap(index, other.index);
            return *this;
        }

        /**
         * \brief Operator overload to act as `element_type&`.
         * \return `element_type&`.
         */
        operator reference()
        { return (ptr->get())[index]; }

        /**
         * \brief Operator overload to act as `const element_type&`.
         * \return `const element_type&`.
         */
        operator const_reference() const
        {
            if (constPtr) return (constPtr->get())[index];
            else return (ptr->get())[index];
        }

        /**
         * \brief Assign operator to assign from another value.
         * \tparam  X       Type of input, `X&` must be implicitly convertible to
         *                  `element_type&`.
         * \param   other   Input value to be assigned from.
         * \details
         *   Used for situations like:
         *   ```cpp
         *   auto ptr = make_shared<int[10]>(0);
         *   ptr[0] = 42;
         *   ```
         */
        template<typename X>
        ArrayHelper& operator=(const X& other)
        {
            operator reference() = other;
            return *this;
        }

    private:
        SafeSharedPtr<T>* ptr = nullptr;
        const SafeSharedPtr<T>* constPtr = nullptr;
        std::ptrdiff_t index = 0;

        ArrayHelper(const ArrayHelper&) = delete;
        ArrayHelper& operator=(const ArrayHelper&) = delete;
    };
#endif

private:
    SafeSharedPtr(std::shared_ptr<ReadWriteLock> l, std::shared_ptr<T> p)
        : lck(l), ptr(p)
    {}

    mutable std::shared_ptr<ReadWriteLock> lck;
    std::shared_ptr<T> ptr;
};

/**
 * \relates SafeSharedPtr
 * \brief Creates a shared pointer that manages a new object.
 * \tparam  T       Type of object to be created.
 * \tparam  Args    Types of arguments in constructor of `T`.
 * \param   args    List of arguments with which an instance of `T` will be
 *                  constructed.
 * \details
 *   Constructs an object of type `T` and wraps it in a `SafeSharedPtr` using `args`
 *   as the parameter list for the constructor of `T`. The object is constructed
 *   as if by the expression `::new (pv) T(std::forward<Args>(args)...)`, where
 *   pv is an internal `void*` pointer to storage suitable to hold an object of
 *   type `T`. The storage is typically larger than `sizeof(T)` in order to use
 *   one allocation for both the control block of the shared pointer and the `T`
 *   object. The `SafeSharedPtr` constructor called by this function enables
 *   ``shared_from_this` with a pointer to the newly constructed object of
 *   type `T`.\n
 *   The object will be destroyed by `p->~X()`, where p is a pointer to the
 *   object and `X` is its type.
 * \exception std::bad_alloc
 *   May throw std::bad_alloc or any exception thrown by the constructor of `T`.
 *   If an exception is thrown, the functions have no effect.
 * \note
 *   A constructor enables `std::enable_shared_from_this` with a pointer ptr of
 *   type `U*` means that it determines if `U` has a base class that is a
 *   specialization of `std::enable_shared_from_this`, and if so, the constructor
 *   evaluates the statement:\n
 *   ```cpp
 *   if (ptr != nullptr && ptr->weak_this.expired())
 *     ptr->weak_this = std::shared_ptr<std::remove_cv_t<U>>(*this,
 *                                      const_cast<std::remove_cv_t<U>*>(ptr));
 *   ```
 *   Where `weak_this` is the hidden mutable `std::weak_ptr` member of
 *   `std::shared_from_this`. The assignment to the `weak_this` member is not
 *   atomic and conflicts with any potentially concurrent access to the same
 *   object. This ensures that future calls to `shared_from_this()` would share
 *   ownership with the `shared_ptr` created by this raw pointer constructor.\n
 *   The test `ptr->weak_this.expired()` in the exposition code above makes sure
 *   that `weak_this` is not reassigned if it already indicates an owner. This
 *   test is required as of C++17.\n
 *   \n
 *   This function may be used as an alternative to
 *   `SafeSharedPtr<T>(new T(args...))`. The trade-offs are: \n
 *     - `SafeSharedPtr<T>(new T(args...))` performs at least two allocations
 *       (one for the object `T` and one for the control block of the shared
 *       pointer), while make_shared<T> typically performs only one allocation
 *       (the standard recommends, but does not require this, all known
 *       implementations do this)\n
 *     - If any SafeWeakPtr references the control block created by make_shared
 *       after the lifetime of all shared owners ended, the memory occupied by
 *       `T` persists until all weak owners get destroyed as well, which may be
 *       undesirable if `sizeof(T)` is large.
 *     - `SafeSharedPtr<T>(new T(args...))` may call a non-public constructor of
 *       `T` if executed in context where it is accessible, while make_shared
 *       requires public access to the selected constructor.\n
 *     - Unlike the `SafeSharedPtr` constructors, make_shared does not allow a
 *       custom deleter.\n
 *     - make_shared uses `::new`, so if any special behavior has been set up
 *       using a class-specific operator new, it will differ from
 *       `SafeSharedPtr<T>(new T(args...))`.\n
 * \sa allocate_shared
 */
template<typename T, typename... Args>
inline SafeSharedPtr<T> make_shared(Args&&... args)
{
    std::shared_ptr<T> p = std::make_shared<T>(std::forward<Args>(args)...);
    return SafeSharedPtr<T>(p);
}

/**
 * \relates SafeSharedPtr
 * \brief Creates a shared pointer that manages a new object allocated using an
 *        allocator.
 * \tparam  T       Type of object to be created.
 * \tparam  Alloc   Type of input allocator.
 * \tparam  Args    Types of arguments in constructor of `T`.
 * \param   alloc   The Allocator to use.
 * \param   args    List of arguments with which an instance of `T` will be
 *                  constructed.
 * \details
 *   Constructs an object of type `T` and wraps it in a `SafeSharedPtr` using args
 *   as the parameter list for the constructor of `T`. The object is constructed
 *   as if by the expression `std::allocator_traits<A2>::construct(a, pv, v)`,
 *   where `pv` is an internal `void*` pointer to storage suitable to hold an
 *   object of type `T` and a is a copy of the allocator rebound to
 *   `std::remove_cv_t<T>`. The storage is typically larger than `sizeof(T)` in
 *   order to use one allocation for both the control block of the shared
 *   pointer and the `T` object. The `SafeSharedPtr` constructor called by this
 *   function enables `shared_from_this` with a pointer to the newly constructed
 *   object of type `T`. All memory allocation is done using a copy of alloc,
 *   which must satisfy the Allocator requirements.\n
 *   For allocate_shared, the object are destroyed via the expression
 *   `std::allocator_traits<A2>::destroy(a, p)`, where `p` is a pointer to the
 *   object and `a` is a copy of the allocator passed to allocate_shared, rebound
 *   to the type of the object being destroyed.
 * \exception UserDefined
 *   Can throw the exceptions thrown from `Alloc::allocate()` or from the
 *   constructor of `T`. If an exception is thrown, this function has no effect.
 * \note
 *   Like make_shared, this function typically performs only one allocation, and
 *   places both the `T` object and the control block in the allocated memory
 *   block (the standard recommends but does not require this, all known
 *   implementations do this). A copy of alloc is stored as part of the control
 *   block so that it can be used to deallocate it once both shared and weak
 *   reference counts reach zero. \n
 *   Unlike the `SafeSharedPtr` constructors, allocate_shared does not accept a
 *   separate custom deleter: the supplied allocator is used for destruction of
 *   the control block and the `T` object, and for deallocation of their shared
 *   memory block.\n
 *   A constructor enables `shared_from_this` with a pointer ptr of type `U*`
 *   means that it determines if `U` has a base class that is a specialization of
 *   `std::enable_shared_from_this`, and if so, the constructor evaluates the
 *   statement:
 *   ```cpp
 *   if (ptr != nullptr && ptr->weak_this.expired())
 *     ptr->weak_this = std::shared_ptr<std::remove_cv_t<U>>(*this,
 *                                      const_cast<std::remove_cv_t<U>*>(ptr));
 *   ```
 *   Where `weak_this` is the hidden mutable `std::weak_ptr` member of
 *   `std::shared_from_this`. The assignment to the `weak_this` member is not
 *   atomic and conflicts with any potentially concurrent access to the same
 *   object. This ensures that future calls to `shared_from_this()` would share
 *   ownership with the `SafeSharedPtr` created by this raw pointer constructor.\n
 *   The test `ptr->weak_this.expired()` in the exposition code above makes sure
 *   that `weak_this` is not reassigned if it already indicates an owner. This
 *   test is required as of C++17.
 * \sa make_shared
 */
template<typename T, typename Alloc, typename... Args>
inline SafeSharedPtr<T> allocate_shared(const Alloc& alloc, Args&&... args)
{
    std::shared_ptr<T> p = std::allocate_shared<T>(alloc, std::forward<Args>(args)...);
    return SafeSharedPtr<T>(p, p.get());
}

/**
 * \relates SafeSharedPtr
 * \brief Applies static_cast to the stored pointer.
 * \tparam  T   Type to cast to.
 * \tparam  U   Type to cast from.
 * \param   r   The pointer to convert.
 * \result SafeSharedPtr<T> casted from type `U`.
 * \details
 *   Creates a new instance of `SafeSharedPtr` whose stored pointer is obtained
 *   from `r`'s stored pointer using a cast expression.\n
 *   If `r` is empty, so is the new `SafeSharedPtr` (but its stored pointer is not
 *   necessarily null). Otherwise, the new `SafeSharedPtr` will share ownership
 *   with the initial value of `r`.\n
 *   Let `Y` be `typename std::shared_ptr<T>::element_type`, then the resulting
 *   SafeSharedPtr's stored pointer will be obtained by evaluating,
 *   respectively: `static_cast<Y*>(r.get())`.\n
 *   The behavior is undefined unless `static_cast<T*>((U*)nullptr)` is well
 *   formed.
 * \note
 *   The expression `SafeSharedPtr<T>(static_cast<T*>(r.get()))` might seem to
 *   have the same effect, but they all will likely result in undefined
 *   behavior, attempting to delete the same object twice!
 */
template<typename T, typename U>
inline SafeSharedPtr<T> static_pointer_cast(const SafeSharedPtr<U>& r) noexcept
{
    auto p = static_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return SafeSharedPtr<T>(r, p);
}

/**
 * \relates SafeSharedPtr
 * \brief Applies dynamic_cast to the stored pointer.
 * \tparam  T   Type to cast to.
 * \tparam  U   Type to cast from.
 * \param   r   The pointer to convert.
 * \result SafeSharedPtr<T> casted from type `U`.
 * \details
 *   Creates a new instance of `SafeSharedPtr` whose stored pointer is obtained
 *   from `r`'s stored pointer using a cast expression.\n
 *   If `r` is empty, so is the new `SafeSharedPtr` (but its stored pointer is not
 *   necessarily null). Otherwise, the new `SafeSharedPtr` will share ownership
 *   with the initial value of `r`, except that it is empty if the `dynamic_cast`
 *   performed by dynamic_pointer_cast returns a null pointer.\n
 *   Let `Y` be `typename std::shared_ptr<T>::element_type`, then the resulting
 *   SafeSharedPtr's stored pointer will be obtained by evaluating,
 *   respectively: `dynamic_cast<Y*>(r.get())` (If the result of the
 *   `dynamic_cast` is a null pointer value, the returned `SafeSharedPtr` will be
 *   empty).
 *   The behavior is undefined unless `dynamic_cast<T*>((U*)nullptr)` is well
 *   formed.
 * \note
 *   The expression `SafeSharedPtr<T>(dynamic_cast<T*>(r.get()))` might seem to
 *   have the same effect, but they all will likely result in undefined
 *   behavior, attempting to delete the same object twice!
 */
template<typename T, typename U>
inline SafeSharedPtr<T> dynamic_pointer_cast(const SafeSharedPtr<U>& r) noexcept
{
    auto p = dynamic_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return SafeSharedPtr<T>(r, p);
}

/**
 * \relates SafeSharedPtr
 * \brief Applies const_cast to the stored pointer.
 * \tparam  T   Type to cast to.
 * \tparam  U   Type to cast from.
 * \param   r   The pointer to convert.
 * \result SafeSharedPtr<T> casted from type `U`.
 * \details
 *   Creates a new instance of `SafeSharedPtr` whose stored pointer is obtained
 *   from `r`'s stored pointer using a cast expression.\n
 *   If `r` is empty, so is the new `SafeSharedPtr` (but its stored pointer is not
 *   necessarily null). Otherwise, the new `SafeSharedPtr` will share ownership
 *   with the initial value of `r`.\n
 *   Let `Y` be `typename std::shared_ptr<T>::element_type`, then the resulting
 *   SafeSharedPtr's stored pointer will be obtained by evaluating,
 *   respectively: `const_cast<Y*>(r.get())`.
 *   The behavior is undefined unless `const_cast<T*>((U*)nullptr)` is well
 *   formed.
 * \note
 *   The expression `SafeSharedPtr<T>(const_cast<T*>(r.get()))` might seem to
 *   have the same effect, but they all will likely result in undefined
 *   behavior, attempting to delete the same object twice!
 */
template<typename T, typename U>
inline SafeSharedPtr<T> const_pointer_cast(const SafeSharedPtr<U>& r) noexcept
{
    auto p = const_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return SafeSharedPtr<T>(r, p);
}

/**
 * \relates SafeSharedPtr
 * \brief Applies reinterpret_cast to the stored pointer.
 * \tparam  T   Type to cast to.
 * \tparam  U   Type to cast from.
 * \param   r   The pointer to convert.
 * \result SafeSharedPtr<T> casted from type `U`.
 * \details
 *   Creates a new instance of `SafeSharedPtr` whose stored pointer is obtained
 *   from `r`'s stored pointer using a cast expression.\n
 *   If `r` is empty, so is the new `SafeSharedPtr` (but its stored pointer is not
 *   necessarily null). Otherwise, the new `SafeSharedPtr` will share ownership
 *   with the initial value of `r`.\n
 *   Let `Y` be `typename std::shared_ptr<T>::element_type`, then the resulting
 *   SafeSharedPtr's stored pointer will be obtained by evaluating,
 *   respectively: `reinterpret_cast<Y*>(r.get())`.
 *   The behavior is undefined unless `reinterpret_cast<T*>((U*)nullptr)` is well
 *   formed.
 * \note
 *   The expression `SafeSharedPtr<T>(reinterpret_cast<T*>(r.get()))` might seem
 *   to have the same effect, but they all will likely result in undefined
 *   behavior, attempting to delete the same object twice!
 */
template<typename T, typename U>
inline SafeSharedPtr<T> reinterpret_pointer_cast(const SafeSharedPtr<U>& r) noexcept
{
    auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type*>(r.get());
    return SafeSharedPtr<T>(r, p);
}

/**
 * \relates SafeSharedPtr
 * \brief Returns the deleter of specified type, if owned.
 * \tparam  Deleter Type of deleter returned.
 * \tparam  T       Type of the object managed by SafeSharedPtr.
 * \param   p       A shared pointer whose deleter needs to be accessed.
 * \return A pointer to the owned deleter or `nullptr`. The returned pointer is
 *         valid at least as long as there remains at least one SafeSharedPtr
 *         instance that owns it.
 * \details
 *   Access to the `p`'s deleter. If the shared pointer `p` owns a deleter of
 *   type cv-unqualified `Deleter` (e.g. if it was created with one of the
 *   constructors that take a deleter as a parameter), then returns a pointer to
 *   the deleter. Otherwise, returns a null pointer.
 * \note
 *   The returned pointer may outlive the last `SafeSharedPtr` if, for example,
 *   SafeWeakPtrs remain and the implementation doesn't destroy the deleter
 *   until the entire control block is destroyed.
 */
template<typename Deleter, typename T>
inline Deleter* get_deleter(const SafeSharedPtr<T>& p) noexcept
{ return std::get_deleter<Deleter>(p.ptr); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `lhs.get() == rhs.get()`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a SafeSharedPtr
 *   created using the aliasing constructor.
 */
template<typename T, typename U>
inline bool operator==(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return lhs.ptr == rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(lhs == rhs)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T, typename U>
inline bool operator!=(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return !(lhs == rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `std::less<V>()(lhs.get(), rhs.get())`, where V is the composite
 *         pointer type of std::SafeSharedPtr<T>::element_type* and
 *         `std::shared_ptr<U>::element_type*`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T, typename U>
inline bool operator<(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return lhs.ptr < rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `rhs < lhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T, typename U>
inline bool operator>(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return lhs.ptr > rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(rhs < lhs)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T, class U>
inline bool operator<=(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return !(lhs > rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(lhs < rhs)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T, class U>
inline bool operator>=(const SafeSharedPtr<T>& lhs, const SafeSharedPtr<U>& rhs) noexcept
{ return !(lhs < rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!lhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T>
inline bool operator==(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return lhs.ptr == rhs; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!rhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T>
inline bool operator==(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return lhs == rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `(bool)lhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T>
inline bool operator!=(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return !(lhs.ptr == rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `(bool)rhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   `SafeSharedPtr` created using the aliasing constructor.
 */
template<typename T>
inline bool operator!=(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return !(lhs == rhs.ptr); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `std::less<SafeSharedPtr<T>::element_type*>()(lhs.get(), nullptr)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator<(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return lhs.ptr < rhs; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `std::less<SafeSharedPtr<T>::element_type*>()(nullptr, rhs.get())`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator<(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return lhs < rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `nullptr < lhs`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator>(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return lhs.ptr > rhs; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `rhs < nullptr`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator>(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return lhs > rhs.ptr; }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(nullptr < lhs)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator<=(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return !(lhs > rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(rhs < nullptr)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator<=(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return !(lhs > rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(lhs < nullptr)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 */
template<typename T>
inline bool operator>=(const SafeSharedPtr<T>& lhs, std::nullptr_t rhs) noexcept
{ return !(lhs < rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Compare `SafeSharedPtr` object with another input.
 * \tparam  T   Type of lhs.
 * \tparam  U   Type of rhs.
 * \param   lhs The left-hand `SafeSharedPtr` to compare.
 * \param   rhs The right-hand `SafeSharedPtr` to compare.
 * \return `!(nullptr < rhs)`.
 * \details
 *   The comparison operators for `SafeSharedPtr` simply compare pointer values;
 *   the actual objects pointed to are not compared. Having operator< defined
 *   for `SafeSharedPtr` allows `SafeSharedPtr` to be used as keys in associative
 *   containers, like `std::map` and `std::set`.
 * \note
 *   In all cases, it is the stored pointer (the one returned by get()) that is
 *   compared, rather than the managed pointer (the one passed to the deleter
 *   when use_count goes to zero). The two pointers may differ in a
 *   SafeSharedPtrcreated using the aliasing constructor.
 * \sa get
 */
template<typename T>
inline bool operator>=(std::nullptr_t lhs, const SafeSharedPtr<T>& rhs) noexcept
{ return !(lhs < rhs); }

/**
 * \relates SafeSharedPtr
 * \brief Outputs the value of the stored pointer to an output stream.
 * \tparam  T   Type of object managed by SafeSharedPtr
 * \tparam  U   First template parameter of ostream.
 * \tparam  V   Second template parameter of ostream.
 * \param   os  A std::basic_ostream to insert ptr into.
 * \param   ptr The data to be inserted into os.
 * \return os
 * \details
 *   Inserts the value of the pointer stored in `ptr` into the output stream
 *   `os`.\n
 *   Equivalent to `os << ptr.get()`.
 */
template<typename T, typename U, typename V>
inline std::basic_ostream<U, V>& operator<<(std::basic_ostream<U, V>& os, const SafeSharedPtr<T>& ptr)
{ return os << ptr.ptr; }

/**
 * \brief Wrapper to `std::weak_ptr` to provide weak reference for SafeSharedPtr.
 * \tparam T type of the object managed by SafeSharedPtr.
 * \details
 *   Same API as `std::weak_ptr`.\n
 *   See https://en.cppreference.com/w/cpp/memory/weak_ptr for more details of
 *   functionalities with std::weak_ptr.\n
 * \note
 *   **Before C++17**, for the purposes of the description below, a pointer type
 *   `Y*` requires that `Y*` must be implicitly convertible to `T*`.\n
 *   **Since C++17**, for the purposes of the description below, a pointer type
 *   `Y*` is said to be **compatible** with a pointer type `T*` if either `Y*` is
 *   convertible to `T*` or `Y` is the array type `U[N]` and `T` is `U cv []` (where
 *   cv is some set of cv-qualifiers).\n
 *   **Since C++17**, default deleter called on destructor will use `delete[]` if
 *   `T` is an arry type;
 * \sa SafeSharedPtr
 */
template<class T>
class SafeWeakPtr
{
public:
    /** \brief Same element_type of SafeSharedPtr. */
    using element_type = typename SafeSharedPtr<T>::element_type;

    /**
     * \brief Default constructor. Constructs empty weak_ptr.
     */
    constexpr SafeWeakPtr() noexcept = default;

    /**
     * \brief Constructs new SafeWeakPtr that shares an object with `other`.
     * \tparam  Y       Element type of input weak pointer.
     * \param   other   Weak pointer to share object from.
     * \details
     *   Constructs new SafeWeakPtr which shares an object managed by `other`. If
     *   `other` manages no object, `*this` manages no object too.
     */
    template<typename Y>
    SafeWeakPtr(const SafeWeakPtr<Y>& other)
        : lck(other.lck), ptr(other.ptr)
    {}

    /**
     * \brief Constructs new SafeWeakPtr that shares an object with `other`.
     * \tparam  Y       Element type of input shared pointer.
     * \param   other   Shared pointer to share object from.
     * \details
     *   Constructs new SafeWeakPtr which shares an object managed by `other`. If
     *   `other` manages no object, `*this` manages no object too.
     */
    template<typename Y>
    SafeWeakPtr(const SafeSharedPtr<Y>& other)
        : lck(other.lck), ptr(other.ptr)
    {}

    /**
     * \brief Destroys the weak_ptr object. Results in no effect to the managed
     *        object.
     */
    ~SafeWeakPtr() = default;

    /**
     * \brief Replaces the managed object with the one managed by `other`.
     * \param other Smart pointer to share an object with.
     * \return `*this`.
     * \details
     *   The object is shared with `other`. If `other` manages no object, `*this`
     *   manages no object too.
     *   Equivalent to `SafeWeakPtr<T>(other).swap(*this)`.
     * \note
     *   The implementation may meet the requirements without creating a
     *   temporary SafeWeakPtr object.
     */
    SafeWeakPtr<T>& operator=(const SafeWeakPtr<T>& other) noexcept
    {
        SafeWeakPtr<T>(other).swap(*this);
        return *this;
    }

    /**
     * \brief Replaces the managed object with the one managed by `other`.
     * \tparam  Y       Element type of input pointer
     * \param   other   Smart pointer to share an object with.
     * \return `*this`.
     * \details
     *   The object is shared with `other`. If `other` manages no object, `*this`
     *   manages no object too.
     *   Equivalent to `SafeWeakPtr<T>(other).swap(*this)`.
     * \note
     *   The implementation may meet the requirements without creating a
     *   temporary SafeWeakPtr object.
     */
    template<typename Y>
    SafeWeakPtr<T>& operator=(const SafeWeakPtr<Y>& other) noexcept
    {
        SafeWeakPtr<T>(other).swap(*this);
        return *this;
    }

    /**
     * \brief Releases the ownership of the managed object. After the call
     *        `*this` manages no object.
     */
    void reset() noexcept
    { SafeWeakPtr<T>().swap(*this); }

    /**
     * \brief Exchanges the contents of `*this` and `other`.
     * \param other Another weak pointer to exchange the contents with.
     * \details
     *   **Complexity**\n
     *   Constant.
     */
    void swap(SafeWeakPtr<T>& other) noexcept
    {
        lck.swap(other.lck);
        ptr.swap(other.ptr);
    }

    /**
     * \brief Returns the number of `SafeSharedPtr` instances that share ownership
     *        of the managed object, or 0 if the managed object has already been
     *        deleted, i.e. `*this` is empty.
     * \return The number of `SafeSharedPtr` instances sharing the ownership of
     *         the managed object at the instant of the call.
     * \note
     *   expired() may be faster than use_count(). This function is inherently
     *   racy, if the managed object is shared among threads that might be
     *   creating and destroying copies of the SafeSharedPtr: then, the result
     *   is reliable only if it matches the number of copies uniquely owned by
     *   the calling thread, or zero; any other value may become stale before
     *   it can be used.
     * \sa expired
     */
    long use_count() const noexcept
    { return ptr.use_count(); }

    /**
     * \brief Checks whether the referenced object was already deleted.
     * \return `true` if the managed object has already been deleted, `false`
     *         otherwise.
     * \brief
     *   Equivalent to `use_count() == 0`. The destructor for the managed object
     *   may not yet have been called, but this object's destruction is imminent
     *   (or may have already happened).
     * \note
     *   This function is inherently racy if the managed object is shared among
     *   threads. In particular, a false result may become stale before it can
     *   be used. A true result is reliable.
     * \sa lock, use_count
     */
    bool expired() const noexcept
    { return ptr.expired(); }

    /**
     * \brief Creates a `SafeSharedPtr` that manages the referenced object.
     * \return A `SafeSharedPtr` which shares ownership of the owned object if
     *         expired returns `false`. Else returns default-constructed
     *         `SafeSharedPtr` of type `T`.
     * \details
     *   Creates a new `SafeSharedPtr` that shares ownership of the managed
     *   object. If there is no managed object, i.e. `*this` is empty, then the
     *   returned `SafeSharedPtr` also is empty.\\n
     *   Effectively returns
     *   `expired() ? SafeSharedPtr<T>() : SafeSharedPtr<T>(*this)`, executed
     *   atomically.
     * \note
     *   Both this function and the constructor of `SafeSharedPtr` may be used to
     *   acquire temporary ownership of the managed object referred to by a
     *   SafeWeakPtr. The difference is that the constructor of SafeSharedPtr
     *   throws an exception when its SafeWeakPtr argument is empty, while
     *   `lock()` constructs an empty SafeSharedPtr<T>.
     */
    SafeSharedPtr<T> lock() const noexcept
    {
        return expired() ? SafeSharedPtr<T>() : SafeSharedPtr<T>(*this);
    }

    /**
     * \brief Provides owner-based ordering of weak pointers.
     * \tparam  Y       Element type of input pointer.
     * \param   other   The SafeWeakPtr to be compared.
     * \return `true` if `*this` precedes other, `false` otherwise. Common
     *         implementations compare the addresses of the control blocks.
     * \details
     *   Checks whether this SafeWeakPtr precedes other in implementation
     *   defined owner-based (as opposed to value-based) order. The order is
     *   such that two smart pointers compare equivalent only if they are both
     *   empty or if they both own the same object, even if the values of the
     *   pointers obtained by get() are different (e.g. because they point at
     *   different subobjects within the same object).\n
     *   This ordering is used to make shared and weak pointers usable as keys
     *   in associative containers, typically through std::owner_less.
     */
    template<typename Y>
    bool owner_before(const SafeWeakPtr<Y>& other) const
    { return ptr.owner_before(other.ptr); }

    /**
     * \brief Provides owner-based ordering of weak pointers.
     * \tparam  Y       Element type of input pointer.
     * \param   other   The `SafeSharedPtr` to be compared.
     * \return `true` if `*this` precedes other, `false` otherwise. Common
     *         implementations compare the addresses of the control blocks.
     * \details
     *   Checks whether this SafeWeakPtr precedes other in implementation
     *   defined owner-based (as opposed to value-based) order. The order is
     *   such that two smart pointers compare equivalent only if they are both
     *   empty or if they both own the same object, even if the values of the
     *   pointers obtained by get() are different (e.g. because they point at
     *   different subobjects within the same object).\n
     *   This ordering is used to make shared and weak pointers usable as keys
     *   in associative containers, typically through std::owner_less.
     */
    template<typename Y>
    bool owner_before(const SafeSharedPtr<Y>& other) const
    { return ptr.owner_before(other); }

private:
    std::weak_ptr<typename SafeSharedPtr<T>::ReadWriteLock> lck;
    std::weak_ptr<T> ptr;
};

/**
 * \brief A proxy class from `std::enable_shared_from_this` to provide same
 *        functionality for SafeSharedPtr.
 * \tparam T Object type same as SafeSharedPtr.
 * \details
 *   This class has no member values, so could be `static_cast` from
 *   `std::enable_shared_from_this` directly.\n
 *   See https://en.cppreference.com/w/cpp/memory/enable_shared_from_this for
 *   more details.
 * \sa SafeSharedPtr
 */
template<typename T>
class EnableSafeSharedFromThis : public std::enable_shared_from_this<T>
{
public:
    /**
     * \brief Constructs a new EnableSafeSharedFromThis object. The private
     *        `std::weak_ptr<T>` member is empty-initialized.
     * \sa SafeSharedPtr
     */
    constexpr EnableSafeSharedFromThis() noexcept
        : __safeSharedLock(std::make_shared<typename SafeSharedPtr<T>::ReadWriteLock>())
    {}

    /**
     * \brief Constructs a new EnableSafeSharedFromThis object. The private
     *        `std::weak_ptr<T>` member is value-initialized.
     * \param other Another EnableSafeSharedFromThis to copy.
     * \note
     *   There is no move constructor: moving from an object derived from
     *   EnableSafeSharedFromThis does not transfer its shared identity.
     * \sa SafeSharedPtr
     */
    EnableSafeSharedFromThis(const EnableSafeSharedFromThis<T>& other) noexcept
        : std::enable_shared_from_this<T>(other),
          __safeSharedLock(other.__safeSharedLock)
    {}

    /**
     * \brief Constructs a new EnableSafeSharedFromThis object. The private
     *        `std::weak_ptr<T>` member is value-initialized.
     * \param other A `std::enable_shared_from_this` to copy.
     * \note
     *   There is no move constructor: moving from an object derived from
     *   EnableSafeSharedFromThis does not transfer its shared identity.
     * \sa SafeSharedPtr
     */
    EnableSafeSharedFromThis(const std::enable_shared_from_this<T>& other) noexcept
        : std::enable_shared_from_this<T>(other),
          __safeSharedLock(std::make_shared<SafeSharedPtr<T>::ReadWriteLock>())
    {}

    /**
     * \brief Destroys `*this`.
     * \sa SafeSharedPtr
     */
    ~EnableSafeSharedFromThis() = default;

    /**
     * \brief Does nothing; returns *this.
     * \param other Another EnableSafeSharedFromThis to assign to `*this`.
     * \return `*this`.
     * \note
     *   The private `std::weak_ptr<T>` member is not affected by this assignment
     *   operator.
     * \sa SafeSharedPtr
     */
    EnableSafeSharedFromThis<T>& operator=(const EnableSafeSharedFromThis<T>& other) noexcept
    {
        static_cast<std::enable_shared_from_this<T>&>(*this)
                = static_cast<std::enable_shared_from_this<T>&>(other);
        __safeSharedLock = other.__safeSharedLock;
        return *this;
    }

    /**
     * \brief Does nothing; returns *this.
     * \param other A `std::enable_shared_from_this` to assign to `*this`.
     * \return `*this`.
     * \note
     *   The private `std::weak_ptr<T>` member is not affected by this assignment
     *   operator.
     * \sa SafeSharedPtr
     */
    EnableSafeSharedFromThis<T>& operator=(const std::enable_shared_from_this<T>& other) noexcept
    {
        static_cast<std::enable_shared_from_this<T>&>(*this) = other;
        __safeSharedLock = std::make_shared<typename SafeSharedPtr<T>::ReadWriteLock>();
        return *this;
    }

    /**
     * \brief Returns a `SafeSharedPtr<T>` that shares ownership of `*this` with
     *        all existing `SafeSharedPtr` that refer to `*this`.
     * \return `SafeSharedPtr<T>` that shares ownership of `*this` with
     *         pre-existing `SafeSharedPtr`s.
     * \details
     *   Effectively executes `SafeSharedPtr<T>(weak_this)`, where `weak_this` is
     *   the private `mutable std::weak_ptr<T>` member of
     *   `std::enable_shared_from_this` base class.
     * \note
     *   It is permitted to call shared_from_this only on a previously shared
     *   object, i.e. on an object managed by `SafeSharedPtr` (in particular,
     *   shared_from_this cannot be called in a constructor).\n
     *   Ohterwise:\n
     *     - **Until C++17**: The behavior is undefined.
     *     - **Since C++17**: `std::bad_weak_ptr` is thrown (by the SafeSharedPtr
     *   constructor from a default-constructed weak_this).
     * \exception std::bad_weak_ptr
     *   **Since C++17**: When enable_shared_from_this called in constructor,
     *   exception will be thrown by the `SafeSharedPtr` constructor from a
     *   default-constructed weak_this.
     * \sa SafeSharedPtr
     */
    SafeSharedPtr<T> shared_from_this()
    {
        return SafeSharedPtr<T>(__safeSharedLock,
                                std::enable_shared_from_this<T>::shared_from_this());
    }

    /**
     * \brief Returns a `SafeSharedPtr<T const>` that shares ownership of `*this`
     *        with all existing `SafeSharedPtr` that refer to `*this`.
     * \return `SafeSharedPtr<T const>` that shares ownership of `*this` with
     *         pre-existing `SafeSharedPtr`s.
     * \details
     *   Effectively executes `SafeSharedPtr<T>(weak_this)`, where `weak_this` is
     *   the private `mutable std::weak_ptr<T>` member of
     *   `std::enable_shared_from_this` base class.
     * \note
     *   It is permitted to call shared_from_this only on a previously shared
     *   object, i.e. on an object managed by `SafeSharedPtr` (in particular,
     *   shared_from_this cannot be called in a constructor).\n
     *   Ohterwise:\n
     *     - **Until C++17**: The behavior is undefined.
     *     - **Since C++17**: `std::bad_weak_ptr` is thrown (by the SafeSharedPtr
     *   constructor from a default-constructed weak_this).
     * \exception std::bad_weak_ptr
     *   **Since C++17**: When enable_shared_from_this called in constructor,
     *   exception will be thrown by the `SafeSharedPtr` constructor from a
     *   default-constructed weak_this.
     * \sa SafeSharedPtr
     */
    SafeSharedPtr<T const> shared_from_this() const
    {
        return SafeSharedPtr<T const>(__safeSharedLock,
                                      std::enable_shared_from_this<T>::shared_from_this());
    }

    /**
     * \brief Returns a `SafeWeakPtr<T>` that tracks ownership of `*this` by all
     *        existing `SafeSharedPtr` that refer to `*this`.
     * \return `SafeWeakPtr<T>` that shares ownership of `*this` with pre-existing
     *         `SafeSharedPtr`s;
     * \note
     *   This is a copy of the the private mutable `std::weak_ptr` member that is
     *   part of `std::enable_shared_from_this`.
     * \sa SafeSharedPtr
     */
    SafeWeakPtr<T> weak_from_this()
    { return shared_from_this(); }

    /**
     * \brief Returns a `SafeWeakPtr<T const>` that tracks ownership of `*this` by
     *        all existing `SafeSharedPtr` that refer to `*this`.
     * \return `SafeWeakPtr<T const>` that shares ownership of `*this` with
     *         pre-existing `SafeSharedPtr`s;
     * \note
     *   This is a copy of the the private mutable `std::weak_ptr` member that is
     *   part of `std::enable_shared_from_this`.
     * \sa SafeSharedPtr
     */
    SafeWeakPtr<T const> weak_from_this() const
    { return shared_from_this(); }

private:
    friend class SafeSharedPtr<T>;
    std::shared_ptr<typename SafeSharedPtr<T>::ReadWriteLock> __safeSharedLock;
};
} // namespace Memory
/** @} end of namespace Memory*/

UTILITIES_NAMESPACE_END

namespace std {
/**
 * \relates Memory::SafeSharedPtr
 * \brief Specializes the `std::swap` algorithm.
 * \tparam  T   Element type of input shared pointers.
 * \param   lhs Shared pointer whose contents to swap.
 * \param   rhs Another shared pointer whose contents to swap.
 * \details
 *   Specializes the `std::swap` algorithm for Memory::SafeSharedPtr. Swaps the
 *   pointers of `lhs` and `rhs`. Calls `lhs.swap(rhs)`.
 * \details
 *   **Complexity**\n
 *   Constant.
 */
template<typename T>
inline void swap(Memory::SafeSharedPtr<T>& lhs, Memory::SafeSharedPtr<T>& rhs) noexcept
{ lhs.swap(rhs); }
/**
 * \relates Memory::SafeWeakPtr
 * \brief Specializes the `std::swap` algorithm.
 * \tparam  T   Element type of input shared pointers.
 * \param   lhs Shared pointer whose contents to swap.
 * \param   rhs Another shared pointer whose contents to swap.
 * \details
 *   Specializes the `std::swap` algorithm for Memory::SafeWeakPtr. Swaps the
 *   pointers of `lhs` and `rhs`. Calls `lhs.swap(rhs)`.
 * \details
 *   **Complexity**\n
 *   Constant.
 */
template<typename T>
void swap(Memory::SafeWeakPtr<T>& lhs, Memory::SafeWeakPtr<T>& rhs) noexcept
{ lhs.swap(rhs); }
} // namespace std

/** @} end of group MemorySafety*/
