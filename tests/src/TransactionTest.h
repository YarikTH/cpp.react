
//          Copyright Sebastian Jeckel 2014.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gtest/gtest.h"

#include <algorithm>
#include <thread>
#include <vector>

#include "react/react.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
namespace {

using namespace react;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// SignalTest fixture
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TParams>
class TransactionTest : public testing::Test
{
public:
    template <EPropagationMode mode>
    class MyEngine : public TParams::template EngineT<mode> {};

    REACTIVE_DOMAIN(MyDomain, TParams::mode, MyEngine)
};

TYPED_TEST_CASE_P(TransactionTest);

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Concurrent transactions test 1
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Concurrent1)
{
    using D = typename Concurrent1::MyDomain;

    std::vector<int> results;

    auto n1 = MakeVar<D>(1);
    auto n2 = n1 + 1;
    auto n3 = n2 + n1 + 1;
    auto n4 = n3 + 1;
    auto n5 = n4 + n3 + n1 + 1;
    auto n6 = n5 + 1;
    auto n7 = n6 + n5 + 1;
    auto n8 = n7 + 1;
    auto n9 = n8 + n7 + n5 + n1 + 1;
    auto n10 = n9 + 1;
    auto n11 = n10 + n9 + 1;
    auto n12 = n11 + 1;
    auto n13 = n12 + n11 + n9 + 1;
    auto n14 = n13 + 1;
    auto n15 = n14 + n13 + 1;
    auto n16 = n15 + 1;
    auto n17 = n16 + n15 + n13 + n9 + 1;

    Observe(n17, [&] (int v)
    {
        results.push_back(v);
    });

    n1 <<= 10;        // 7732
    n1 <<= 100;        // 68572
    n1 <<= 1000;    // 676972

    ASSERT_EQ(results.size(), 3);

    ASSERT_TRUE(std::find(results.begin(), results.end(), 7732) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 68572) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 676972) != results.end());

    // Reset
    n1 <<= 1;
    results.clear();

    // Now do the same from 3 threads

    std::thread t1([&]    { n1 <<= 10; });
    std::thread t2([&]    { n1 <<= 100; });
    std::thread t3([&]    { n1 <<= 1000; });

    t1.join();
    t2.join();
    t3.join();

    ASSERT_EQ(results.size(), 3);

    ASSERT_TRUE(std::find(results.begin(), results.end(), 7732) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 68572) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 676972) != results.end());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Concurrent transactions test 2
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Concurrent2)
{
    using D = typename Concurrent2::MyDomain;

    std::vector<int> results;

    auto in = MakeVar<D>(-1);

    // 1. Generate graph
    Signal<D,int> n0 = in;

    auto next = n0;

    for (int i=0; i < 100; i++)
    {
        auto q = next + 0;
        next = q;
    }

    Observe(next, [&] (int v)
    {
        results.push_back(v);
    });

    // 2. Send events
    std::thread t1([&]
    {
        for (int i=0; i<100; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 50));
            in <<= i;
        }
    });

    std::thread t2([&]
    {
        for (int i=100; i<200; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 50));
            in <<= i;
        }
    });

    std::thread t3([&]
    {
        for (int i=200; i<300; i++)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 50));
            in <<= i;
        }
    });

    t1.join();
    t2.join();
    t3.join();

    ASSERT_EQ(results.size(), 300);

    for (int i=0; i<300; i++)
    {
        ASSERT_TRUE(std::find(results.begin(), results.end(), i) != results.end());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Concurrent transactions test 3
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Concurrent3)
{
    using D = typename Concurrent3::MyDomain;

    std::vector<int> results;

    std::function<int(int)> f_0 = [] (int a) -> int
    {
        for (int i = 0; i<(a+1)*100; i++);
        return a + 1;
    };

    std::function<int(int,int)> f_n = [] (int a, int b) -> int
    {
        for (int i = 0; i<(a+b)*100; i++);
        return a + b;
    };

    auto n1 = MakeVar<D>(1);
    auto n2 = n1 ->* f_0;
    auto n3 = ((n2, n1) ->* f_n) ->* f_0;
    auto n4 = n3 ->* f_0;
    auto n5 = ((((n4, n3) ->* f_n), n1) ->* f_n) ->* f_0;
    auto n6 = n5 ->* f_0;
    auto n7 = ((n6, n5) ->* f_n) ->* f_0;
    auto n8 = n7 ->* f_0;
    auto n9 = ((((((n8, n7) ->* f_n), n5) ->* f_n), n1) ->* f_n) ->* f_0;
    auto n10 = n9 ->* f_0;
    auto n11 = ((n10, n9) ->* f_n) ->* f_0;
    auto n12 = n11 ->* f_0;
    auto n13 = ((((n12, n11) ->* f_n), n9) ->* f_n) ->* f_0;
    auto n14 = n13 ->* f_0;
    auto n15 = ((n14, n13) ->* f_n) ->* f_0;
    auto n16 = n15 ->* f_0;
    auto n17 = ((((((n16, n15) ->* f_n), n13) ->* f_n), n9) ->* f_n)  ->* f_0;

    Observe(n17, [&] (int v)
    {
        results.push_back(v);
    });

    n1 <<= 1000;    // 676972
    n1 <<= 100;     // 68572
    n1 <<= 10;      // 7732


    ASSERT_EQ(results.size(), 3);

    ASSERT_TRUE(std::find(results.begin(), results.end(), 7732) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 68572) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 676972) != results.end());

    // Reset
    n1 <<= 1;
    results.clear();
    
    std::thread t3([&]    { n1 <<= 1000; });
    std::thread t2([&]    { n1 <<= 100; });
    std::thread t1([&]    { n1 <<= 10; });
    
    t3.join();
    t2.join();
    t1.join();


    ASSERT_EQ(results.size(), 3);

    ASSERT_TRUE(std::find(results.begin(), results.end(), 7732) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 68572) != results.end());
    ASSERT_TRUE(std::find(results.begin(), results.end(), 676972) != results.end());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Merging1
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Merging1)
{
    using D = typename Merging1::MyDomain;

    std::vector<int> results;

    std::atomic<bool> shouldSpin(false);

    std::function<int(int)> f = [&shouldSpin] (int a) -> int
    {
        while (shouldSpin);

        return a;
    };

    auto n1 = MakeVar<D>(1);
    auto n2 = n1 ->* f;

    Observe(n2, [&] (int v) {
        results.push_back(v);
    });

    // Todo: improve this as it'll fail occasionally
    shouldSpin = true;
    std::thread t1([&] {
        DoTransaction<D>(allow_merging, [&] {
            n1 <<= 2;
        });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::thread t2([&] {
        DoTransaction<D>(allow_merging, [&] {
            n1 <<= 3;
        });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread t3([&] {
        DoTransaction<D>(allow_merging, [&] {
            n1 <<= 4;
        });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::thread t4([&] {
        DoTransaction<D>(allow_merging, [&] {
            n1 <<= 5;
        });
        
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    shouldSpin = false;

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    ASSERT_EQ(results.size(), 2);
    ASSERT_TRUE(results[0] == 2);
    ASSERT_TRUE(results[1] == 5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Async1
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Async1)
{
    using D = typename Async1::MyDomain;

    std::vector<int> results;

    auto in = MakeVar<D>(1);
    auto s1 = in * 1000;

    Observe(s1, [&] (int v) {
        results.push_back(v);
    });

    TransactionStatus st;

    AsyncTransaction<D>(st, [&] {
        in <<= 10;
    });

    AsyncTransaction<D>(st, [&] {
        in <<= 20;
    });

    AsyncTransaction<D>(st, [&] {
        in <<= 30;
    });

    st.Wait();

    ASSERT_EQ(results.size(), 3);
    ASSERT_TRUE(results[0] == 10000);
    ASSERT_TRUE(results[1] == 20000);
    ASSERT_TRUE(results[2] == 30000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// AsyncMerging1
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, AsyncMerging1)
{
    using D = typename AsyncMerging1::MyDomain;

    std::vector<int> results;

    auto in = MakeVar<D>(1);
    auto s1 = in * 1000;

    Observe(s1, [&] (int v) {
        results.push_back(v);
    });

    TransactionStatus st;

    AsyncTransaction<D>(allow_merging, st, [&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        in <<= 10;
    });

    // Make sure other async transaction gets to start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // These two can still be pulled in after the first input function is done
    AsyncTransaction<D>(allow_merging, st, [&] {
        in <<= 20;
    });

    AsyncTransaction<D>(allow_merging, st, [&] {
        in <<= 30;
    });

    // Can't be merged
    AsyncTransaction<D>(st, [&] {
        in <<= 40;
    });

    // These two should be merged again
    AsyncTransaction<D>(allow_merging, st, [&] {
        in <<= 50;
    });

    AsyncTransaction<D>(allow_merging, st, [&] {
        in <<= 60;
    });

    st.Wait();

    ASSERT_EQ(results.size(), 3);
    ASSERT_TRUE(results[0] == 30000);
    ASSERT_TRUE(results[1] == 40000);
    ASSERT_TRUE(results[2] == 60000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Continuation1
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Continuation1)
{
    using D = typename Continuation1::MyDomain;

    std::vector<int> results;

    auto in = MakeVar<D>(0);

    auto cont = MakeContinuation(in, [&] (int v) {
        results.push_back(v);

        if (v < 10)
            in <<= v + 1;
    });

    in <<= 1;

    ASSERT_EQ(results.size(), 10);
    for (int i=0; i<10; i++)
        ASSERT_TRUE(results[i] == i+1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Continuation2
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Continuation2)
{
    using L = typename Continuation2::MyDomain;

    REACTIVE_DOMAIN(R, sequential_concurrent)

    std::vector<int> results;

    auto srcL = MakeVar<L>(0);
    auto srcR = MakeVar<R>(0);

    auto contL = MakeContinuation<L,R>(srcL, [&] (int v) {
        results.push_back(v);
        if (v < 10)
            srcR <<= v+1;
    });

    auto contR = MakeContinuation<R,L>(Monitor(srcR), [&] (int v) {
        results.push_back(v);
        if (v < 10)
            srcL <<= v+1;
    });

    srcL <<= 1;

    ASSERT_EQ(results.size(), 10);
    for (int i=0; i<10; i++)
        ASSERT_TRUE(results[i] == i+1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Continuation3
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Continuation3)
{
    using L = typename Continuation3::MyDomain;

    REACTIVE_DOMAIN(R, sequential_concurrent)

    std::vector<int> results;

    auto srcL = MakeVar<L>(0);
    auto depL1 = MakeVar<L>(100);
    auto depL2 = MakeVar<L>(10);
    auto srcR = MakeVar<R>(0);

    auto contL = MakeContinuation<L,R>(
        Monitor(srcL),
        With(depL1, depL2),
        [&] (int v, int depL1, int depL2) {
            ASSERT_EQ(depL1, v*100);
            ASSERT_EQ(depL2, v*10);
            results.push_back(v);
            if (v < 10)
                srcR <<= v+1;
        });

    auto contR = MakeContinuation<R,L>(
        Monitor(srcR),
        [&] (int v) {
            results.push_back(v);

            v++;
            depL1 <<= v*100;
            depL2 <<= v*10;
            if (v < 10)
                srcL <<= v;
        });

    srcL <<= 1;

    ASSERT_EQ(results.size(), 10);
    for (int i=0; i<10; i++)
        ASSERT_TRUE(results[i] == i+1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Continuation4 test
///////////////////////////////////////////////////////////////////////////////////////////////////
TYPED_TEST_P(TransactionTest, Continuation4)
{
    using D = typename Continuation4::MyDomain;

    using std::vector;

    auto vect = MakeVar<D>(vector<int>{});

    int count = 0;

    auto cont = MakeContinuation(vect, [&] (const vector<int>& v)
    {    
        if (count == 0)
        {
            ASSERT_EQ(v[0], 30);

            vect.Modify([] (vector<int>& v) {
                v.push_back(50);
            });
        }
        else if (count == 1)
        {
            ASSERT_EQ(v[1], 50);

            vect.Modify([] (vector<int>& v) {
                v.push_back(70);
            });
        }
        else
        {
            ASSERT_EQ(v[2], 70);
        }

        count++;
    });

    vect.Modify([] (vector<int>& v) {
        v.push_back(30);
    });

    ASSERT_EQ(count, 3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
REGISTER_TYPED_TEST_CASE_P
(
    TransactionTest,
    Concurrent1,
    Concurrent2,
    Concurrent3,
    Merging1,
    Async1,
    AsyncMerging1,
    Continuation1,
    Continuation2,
    Continuation3,
    Continuation4
);

} // ~namespace
