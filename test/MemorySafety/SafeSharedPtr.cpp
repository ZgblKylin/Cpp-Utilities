#include <gtest/gtest.h>
#include <thread>
#include <functional>
#include <string>
#include <sstream>
#define private public
#include <MemorySafety/SafeSharedPtr.hpp>

UTILITIES_USING_NAMESPACE;
using Memory::SafeSharedPtr;
using Memory::SafeWeakPtr;

struct Base
{
    Base(int x) : i(x) {}
    virtual ~Base() = default;
    int i;
};

struct Delivered : public Base
{
    using Base::Base;
};

TEST(SafeSharedPtr, Constructor)
{
    auto defaulted = SafeSharedPtr<int>();
    EXPECT_TRUE(defaulted.lck);
    EXPECT_FALSE(defaulted.ptr);

    auto nullPtr = SafeSharedPtr<int>(nullptr);
    EXPECT_TRUE(nullPtr.lck);
    EXPECT_FALSE(nullPtr.ptr);

    auto rawPointer = SafeSharedPtr<int>(new int(3));
    EXPECT_TRUE(rawPointer.lck);
    EXPECT_EQ(*rawPointer.ptr, 3);

    bool deleted = false;
    {
        auto rawDeleter = SafeSharedPtr<int>(new int(3), [&deleted](int* p){
            if(p) delete p;
            deleted = true;
        });
        EXPECT_TRUE(rawDeleter.lck);
        EXPECT_EQ(*rawDeleter.ptr, 3);
    }
    EXPECT_TRUE(deleted);

    deleted = false;
    {
        auto nullDeleter = SafeSharedPtr<int>(nullptr, [&deleted](int* p){
            if(p) delete p;
            deleted = true;
        });
        EXPECT_TRUE(nullDeleter.lck);
        EXPECT_FALSE(nullDeleter.ptr);
    }
    EXPECT_TRUE(deleted);

    deleted = false;
    {
        auto rawAllocator = SafeSharedPtr<int>(
                                new int(3),
                                [&deleted](int* p){ if(p) { delete p; } deleted = true; },
        std::allocator<int>()
        );
        EXPECT_TRUE(rawAllocator.lck);
        EXPECT_EQ(*rawAllocator.ptr, 3);
    }
    EXPECT_TRUE(deleted);

    deleted = false;
    {
        auto nullAllocator = SafeSharedPtr<int>(
                                 nullptr,
                                 [&deleted](int* p){ if(p) { delete p; } deleted = true; },
                                 std::allocator<int>()
        );
        EXPECT_TRUE(nullAllocator.lck);
        EXPECT_FALSE(nullAllocator.ptr);
    }
    EXPECT_TRUE(deleted);

    deleted = false;
    {
        auto other = SafeSharedPtr<int>(new int(3), [&deleted](int* p){
            if(p) { delete p; }
            deleted = true;
        });
        auto otherPtr = SafeSharedPtr<int>(other, new int(4));
        EXPECT_EQ(*otherPtr.ptr, 4);
        other.reset();
        EXPECT_FALSE(deleted);
        EXPECT_EQ(otherPtr.ptr.use_count(), 1);
    }
    EXPECT_TRUE(deleted);

    deleted = false;
    {
        auto other = std::shared_ptr<int>(new int(3), [&deleted](int* p){
            if(p) { delete p; }
            deleted = true;
        });
        auto otherPtr = SafeSharedPtr<int>(other, new int(4));
        EXPECT_EQ(*otherPtr.ptr, 4);
        other.reset();
        EXPECT_FALSE(deleted);
        EXPECT_EQ(otherPtr.ptr.use_count(), 1);
    }
    EXPECT_TRUE(deleted);

    {
        auto other = SafeSharedPtr<int>(new int(3));
        SafeSharedPtr<int> copy(other);
        EXPECT_EQ(*copy.ptr, 3);

        auto otherY = SafeSharedPtr<Delivered>(new Delivered(3));
        SafeSharedPtr<Base> copyY(otherY);
        EXPECT_EQ(copyY.ptr->i, 3);
    }

    {
        SafeSharedPtr<int> copy(SafeSharedPtr<int>(new int(3)));
        EXPECT_EQ(*copy.ptr, 3);

        SafeSharedPtr<Base> copyY(SafeSharedPtr<Delivered>(new Delivered(3)));
        EXPECT_EQ(copyY.ptr->i, 3);
    }

    {
        auto other = std::shared_ptr<int>(new int(3));
        SafeSharedPtr<int> copy(other);
        EXPECT_EQ(*copy.ptr, 3);

        auto otherY = std::shared_ptr<Delivered>(new Delivered(3));
        SafeSharedPtr<Base> copyY(otherY);
        EXPECT_EQ(copyY.ptr->i, 3);
    }

    {
        SafeSharedPtr<int> copy(std::shared_ptr<int>(new int(3)));
        EXPECT_EQ(*copy.ptr, 3);

        SafeSharedPtr<Base> copyY(std::shared_ptr<Delivered>(new Delivered(3)));
        EXPECT_EQ(copyY.ptr->i, 3);
    }

    {
        SafeSharedPtr<int> ptr(new int(3));
        SafeWeakPtr<int> weak(ptr);
        EXPECT_EQ(*SafeSharedPtr<int>(weak).ptr, 3);
    }

    {
        std::shared_ptr<int> ptr(new int(3));
        std::weak_ptr<int> weak(ptr);
        EXPECT_EQ(*SafeSharedPtr<int>(weak).ptr, 3);
    }
}

TEST(SafeSharedPtr, assignment)
{
    {
        SafeSharedPtr<int> ptr(new int(3));
        auto other = ptr;
        EXPECT_EQ(*other.ptr, 3);

        other = SafeSharedPtr<int>(new int(4));
        EXPECT_EQ(*other.ptr, 4);
    }

    {
        SafeSharedPtr<Delivered>  ptr(new Delivered(3));
        SafeSharedPtr<Base> other = ptr;
        EXPECT_EQ(other.ptr->i, 3);

        other = SafeSharedPtr<Delivered>(new Delivered(4));
        EXPECT_EQ(other.ptr->i, 4);
    }

    {
        std::shared_ptr<Delivered>  ptr(new Delivered(3));
        SafeSharedPtr<Base> other = ptr;
        EXPECT_EQ(other.ptr->i, 3);

        other = std::shared_ptr<Delivered>(new Delivered(4));
        EXPECT_EQ(other.ptr->i, 4);
    }
}

TEST(SafeSharedPtr, reset)
{
    SafeSharedPtr<Base> ptr(new Base(3));
    EXPECT_EQ(ptr.ptr->i, 3);

    ptr.reset();
    EXPECT_FALSE(ptr.ptr);

    ptr.reset(new Delivered(3));
    EXPECT_EQ(ptr.ptr->i, 3);

    bool deleted = false;
    ptr.reset(new Delivered(4), [&deleted](Base* p){
        delete p;
        deleted = true;
    });
    EXPECT_EQ(ptr.ptr->i, 4);
    ptr = nullptr;
    EXPECT_FALSE(ptr.ptr);
    EXPECT_TRUE(deleted);
}

TEST(SafeSharedPtr, swap)
{
    SafeSharedPtr<int> ptr1(new int(3));
    SafeSharedPtr<int> ptr2(new int(4));

    ptr1.swap(ptr2);
    EXPECT_EQ(*ptr1.ptr, 4);
    EXPECT_EQ(*ptr2.ptr, 3);

    std::swap(ptr1, ptr2);
    EXPECT_EQ(*ptr1.ptr, 3);
    EXPECT_EQ(*ptr2.ptr, 4);
}

TEST(SafeSharedPtr, dataAccess)
{
    SafeSharedPtr<Base> ptr(new Base(3));
    EXPECT_EQ((*(ptr.get())).i, 3);

    SafeSharedPtr<int> pInt(new int(3));
    EXPECT_EQ(*pInt, 3);

    EXPECT_EQ(ptr->i, 3);

#if __cplusplus >= 201703L
    SafeSharedPtr<int[]> pArray(new int[2]{1, 2});
    pArray[0] = 1;
    pArray[1] = 2;
    EXPECT_EQ(pArray.get()[0], 1);
    EXPECT_EQ(pArray.get()[1], 2);
#endif
}

TEST(SafeSharedPtr, use_count)
{
    SafeSharedPtr<int> ptr(new int(3));
    EXPECT_EQ(ptr.use_count(), 1);

    auto ptr2 = ptr;
    EXPECT_EQ(ptr.use_count(), 2);

    ptr.reset();
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST(SafeSharedPtr, owner_before)
{
    std::shared_ptr<Base> p1(new Delivered(3));
    std::shared_ptr<Delivered> p2(new Delivered(4));
    bool before = p1.owner_before(p2);

    SafeSharedPtr<Base> ptr1(p1);
    SafeSharedPtr<Delivered> ptr2(p2);
    ASSERT_EQ(ptr1.owner_before(ptr2), before);

    SafeWeakPtr<Delivered> weak(ptr2);
    ASSERT_EQ(ptr1.owner_before(weak), before);
}

TEST(SafeSharedPtr, concurrent)
{
    SafeSharedPtr<int> ptr(new int(0));
    int sum;
    std::thread thread([&sum](SafeSharedPtr<int> ptr) {
        for (int i = 0; i < 100 * 1000; ++i)
        {
            const auto& cPtr = ptr;
            sum = *cPtr;

            for (int j = 0; j < 10; ++j)
            { *ptr += 1; }
        }
    }, ptr);
    for (int i = 0; i < 100 * 1000; ++i)
    {
        const auto& cPtr = ptr;
        sum = *cPtr;

        for (int j = 0; j < 10; ++j)
        { *ptr += 1; }
    }
    thread.join();
    EXPECT_TRUE(sum >= 2 * 100 * 1000 * 9);
    EXPECT_EQ(*ptr, 2 * 100 * 1000 * 10);
}

TEST(SafeSharedPtr, lock)
{
    SafeSharedPtr<int> ptr(new int(0));
    int sum;
    std::thread thread([&sum](SafeSharedPtr<int> ptr){
        for (int i = 0; i < 100 * 1000; ++i)
        {
            ptr.lock_shared();
            sum = *(ptr.get());
            ptr.unlock_shared();

            ptr.lock();
            for (int j = 0; j < 10; ++j)
            { *(ptr.get()) += 1; }
            ptr.unlock();
        }
    }, ptr);
    for (int i = 0; i < 100 * 1000; ++i)
    {
        ptr.lock_shared();
        sum = *(ptr.get());
        ptr.unlock_shared();

        ptr.lock();
        for (int j = 0; j < 10; ++j)
        { *(ptr.get()) += 1; }
        ptr.unlock();
    }
    thread.join();
    EXPECT_TRUE(sum >= 2 * 100 * 1000 * 9);
    EXPECT_EQ(*ptr, 2 * 100 * 1000 * 10);
}

TEST(SafeSharedPtr, lock_guard)
{
#if __cplusplus >= 201703L
    SafeSharedPtr<int> ptr(new int(0));
    int sum;
    std::thread thread([&sum](SafeSharedPtr<int> ptr){
        for (int i = 0; i < 100 * 1000; ++i)
        {
            {
                SafeSharedPtr<int>::SharedLock shared_lock = ptr.shared_lock();
                sum = *(ptr.get());
            }

            auto unique_lock = ptr.unique_lock();
            for (int j = 0; j < 10; ++j)
            { *(ptr.get()) += 1; }
        }
    }, ptr);
    for (int i = 0; i < 100 * 1000; ++i)
    {
        {
            SafeSharedPtr<int>::SharedLock shared_lock = ptr.shared_lock();
            sum = *(ptr.get());
        }

        auto unique_lock = ptr.unique_lock();
        for (int j = 0; j < 10; ++j)
        { *(ptr.get()) += 1; }
    }
    thread.join();
    EXPECT_TRUE(sum >= 2 * 100 * 1000 * 9);
    EXPECT_EQ(*ptr, 2 * 100 * 1000 * 10);
#endif
}

TEST(SafeSharedPtr, make_shared)
{
    SafeSharedPtr<int> ptr = Memory::make_shared<int>(3);
    ASSERT_TRUE(ptr);
    EXPECT_EQ(*ptr, 3);
}

TEST(SafeSharedPtr, pointer_cast)
{
    SafeSharedPtr<Delivered> delivered(new Delivered(3));
    SafeSharedPtr<Base> base = Memory::static_pointer_cast<Base>(delivered);
    ASSERT_TRUE(base);
    EXPECT_EQ(base->i, 3);

    SafeSharedPtr<Delivered> delivered2 = Memory::dynamic_pointer_cast<Delivered>(base);
    ASSERT_TRUE(delivered2);
    EXPECT_EQ(delivered2->i, 3);

    SafeSharedPtr<const int> constInt(new int(3));
    SafeSharedPtr<int> pInt = Memory::const_pointer_cast<int>(constInt);
    ASSERT_TRUE(pInt);
    EXPECT_EQ(*pInt, 3);

    float a(3.14);
    int b = *reinterpret_cast<int*>(&a);
    SafeSharedPtr<float> pFloat(new float(a));
    pInt = Memory::reinterpret_pointer_cast<int>(pFloat);
    EXPECT_EQ(*pInt, b);
}

static bool get_deleter_deleted = false;
void deleter(Base* p)
{
    delete p;
    get_deleter_deleted = true;
}

TEST(SafeSharedPtr, get_deleter)
{
    struct RAII {
        RAII(std::function<void(void)> onDestruction)
            : onDestruct(onDestruction)
        {}
        ~RAII() { onDestruct(); }
        std::function<void(void)> onDestruct;
    };
    RAII raii([]{ ASSERT_TRUE(get_deleter_deleted); });

    SafeSharedPtr<int> aliasPtr;
    {
        // create a shared_ptr that owns a Base and a deleter
        SafeSharedPtr<Base> ptr(new Base(3), deleter);
        aliasPtr = SafeSharedPtr<int>(ptr, &ptr->i); // aliasing constructor
        // aliasPtr is now pointing to an int, but managing the whole Base
    } // ptr gets destroyed (deleter not called)
    ASSERT_TRUE(aliasPtr);
    EXPECT_EQ(*aliasPtr, 3);

    // obtain pointer to the deleter:
    auto del_p = Memory::get_deleter<void(*)(Base*)>(aliasPtr);
    EXPECT_TRUE(*del_p == deleter);
}

TEST(SafeSharedPtr, comparison)
{
    SafeSharedPtr<Delivered> a(new Delivered(3));
    SafeSharedPtr<Base> b = a;
    SafeSharedPtr<Base> c(new Delivered(4));

    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_EQ(a > c, a.ptr > c.ptr);
    EXPECT_EQ(a < c, a.ptr < c.ptr);
    EXPECT_EQ(a >= c, a.ptr >= c.ptr);
    EXPECT_EQ(a <= c, a.ptr <= c.ptr);

    EXPECT_FALSE(a == nullptr);
    EXPECT_FALSE(nullptr == a);
    EXPECT_TRUE(a != nullptr);
    EXPECT_TRUE(nullptr != a);
    EXPECT_EQ(a < nullptr, a.ptr < nullptr);
    EXPECT_EQ(nullptr < a, nullptr < a);
    EXPECT_EQ(a > nullptr, a.ptr > nullptr);
    EXPECT_EQ(nullptr > a, nullptr > a);
    EXPECT_EQ(a <= nullptr, a.ptr <= nullptr);
    EXPECT_EQ(nullptr <= a, nullptr <= a.ptr);
    EXPECT_EQ(a >= nullptr, a.ptr >= nullptr);
    EXPECT_EQ(nullptr >= a, nullptr >= a.ptr);
}

TEST(SafeSharedPtr, stream)
{
    SafeSharedPtr<int> ptr(new int(3));
    std::stringstream buf1;
    buf1 << ptr;
    std::stringstream buf2;
    buf2 << ptr.ptr;
    EXPECT_EQ(buf1.str(), buf2.str());
}

TEST(SafeSharedPtr, weakPtr)
{
    SafeSharedPtr<Delivered> ptr(new Delivered(3));
    SafeWeakPtr<Delivered> weak(ptr);
    SafeWeakPtr<Base> weakBase(weak);
    ASSERT_FALSE(weakBase.expired());
    EXPECT_EQ(weakBase.use_count(), 1);
    EXPECT_EQ(weakBase.lock()->i, 3);
    EXPECT_EQ(weakBase.owner_before(weak),
              weakBase.ptr.owner_before(weak.ptr));

    ptr.reset();
    ASSERT_TRUE(weakBase.expired());

    SafeSharedPtr<Delivered> ptr2(new Delivered(4));
    SafeWeakPtr<Delivered> weak2(ptr2);
    SafeWeakPtr<Base>weakBase2;

    weakBase2 = weak2;
    ASSERT_FALSE(weakBase2.expired());
    EXPECT_EQ(weakBase2.lock()->i, 4);

    weakBase2 = weakBase;
    EXPECT_TRUE(weakBase.expired());

    weak.swap(weak2);
    ASSERT_FALSE(weak.expired());
    EXPECT_EQ(weak.lock()->i, 4);

    std::swap(weak, weak2);
    ASSERT_TRUE(weak.expired());
    EXPECT_EQ(weak2.lock()->i, 4);
}

struct Good : public Delivered, public Memory::EnableSafeSharedFromThis<Good>
{
    Good(int x = 0) : Delivered(x) {}

    SafeSharedPtr<Good> getptr()
    { return shared_from_this(); }
};

TEST(SafeSharedPtr, EnableSafeSharedFromThis)
{
    SafeSharedPtr<Good> gp1 = Memory::make_shared<Good>();
    SafeSharedPtr<Good> gp2 = gp1->getptr();
    EXPECT_EQ(gp2.use_count(), 2);
    EXPECT_EQ(gp2.lck.use_count(), 3);

    SafeSharedPtr<Good> ptr(new Good(3));
    EXPECT_EQ(ptr.lck.use_count(), 2);
    EXPECT_EQ(ptr->i, 3);

    bool deleted = false;
    SafeSharedPtr<Good> ptr2(new Good(3), [&deleted](Good* p){
        delete p;
        deleted = true;
    });
    EXPECT_EQ(ptr2.lck.use_count(), 2);
    EXPECT_EQ(ptr2->i, 3);
    ptr2.reset();
    EXPECT_TRUE(deleted);

    deleted = false;
    SafeSharedPtr<Good> ptr3(new Good(3), [&deleted](Good* p){
        delete p;
        deleted = true;
    }, std::allocator<Good>());
    EXPECT_EQ(ptr3.lck.use_count(), 2);
    EXPECT_EQ(ptr3->i, 3);
    ptr3.reset();
    EXPECT_TRUE(deleted);

    std::shared_ptr<Good> p = std::make_shared<Good>(3);
    Good* good = new Good(4);
    SafeSharedPtr<Good> ptr4(p, good);
    EXPECT_EQ(p->__safeSharedLock.use_count(), 2);
    EXPECT_EQ(ptr4.lck.use_count(), 2);
    EXPECT_EQ(good->__safeSharedLock.use_count(), 1);
    EXPECT_EQ(ptr4->i, 4);

    SafeSharedPtr<Good> ptr5(p);
    EXPECT_EQ(ptr5.lck.use_count(), 3);
    EXPECT_EQ(ptr5->i, 3);

    SafeSharedPtr<Good> ptr6(std::make_shared<Good>(3));
    EXPECT_EQ(ptr6.lck.use_count(), 2);
    EXPECT_EQ(ptr6->i, 3);
}
