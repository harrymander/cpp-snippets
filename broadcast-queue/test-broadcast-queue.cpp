#include <gtest/gtest.h>

#include "broadcast-queue.hpp"

using namespace recap::app::broadcast_queue;

#define Test(name) TEST(TestBroadcastQueue, test_##name)

Test(num_subscribers)
{
    BroadcastQueue<int> queue;
    ASSERT_EQ(queue.num_subscribers(), 0);
    {
        queue.subscribe();
        ASSERT_EQ(queue.num_subscribers(), 0);

        auto sub1 = queue.subscribe();
        ASSERT_EQ(queue.num_subscribers(), 1);
        auto sub2 = queue.subscribe();
        ASSERT_EQ(queue.num_subscribers(), 2);

        {
            auto sub3 = queue.subscribe();
            ASSERT_EQ(queue.num_subscribers(), 3);
        }
        ASSERT_EQ(queue.num_subscribers(), 2);
    }
    ASSERT_EQ(queue.num_subscribers(), 0);
}

Test(queuing)
{
    BroadcastQueue<int> queue;
    queue.push(0);

    auto sub1 = queue.subscribe();
    ASSERT_TRUE(sub1.empty());
    ASSERT_EQ(sub1.size(), 0);

    queue.push(1);
    ASSERT_EQ(sub1.size(), 1);
    ASSERT_FALSE(sub1.empty());
    ASSERT_EQ(sub1.back(), 1);
    ASSERT_EQ(sub1.front(), 1);

    queue.push(2);
    ASSERT_EQ(sub1.size(), 2);
    ASSERT_FALSE(sub1.empty());
    ASSERT_EQ(sub1.back(), 2);
    ASSERT_EQ(sub1.front(), 1);

    auto sub2 = queue.subscribe();
    queue.push(3);
    ASSERT_EQ(sub1.size(), 3);
    ASSERT_FALSE(sub1.empty());
    ASSERT_EQ(sub1.back(), 3);
    ASSERT_EQ(sub1.front(), 1);
    ASSERT_EQ(sub2.size(), 1);
    ASSERT_FALSE(sub2.empty());
    ASSERT_EQ(sub2.back(), 3);
    ASSERT_EQ(sub2.front(), 3);

    {
        auto sub3 = queue.subscribe();
        queue.push(4);
        ASSERT_EQ(sub1.size(), 4);
        ASSERT_FALSE(sub1.empty());
        ASSERT_EQ(sub1.back(), 4);
        ASSERT_EQ(sub1.front(), 1);
        ASSERT_EQ(sub2.size(), 2);
        ASSERT_FALSE(sub2.empty());
        ASSERT_EQ(sub2.back(), 4);
        ASSERT_EQ(sub2.front(), 3);
        ASSERT_EQ(sub3.size(), 1);
        ASSERT_FALSE(sub3.empty());
        ASSERT_EQ(sub3.back(), 4);
        ASSERT_EQ(sub3.front(), 4);
    }

    ASSERT_EQ(sub1.size(), 4);
    ASSERT_FALSE(sub1.empty());
    ASSERT_EQ(sub1.back(), 4);
    ASSERT_EQ(sub1.front(), 1);
    ASSERT_EQ(sub2.size(), 2);
    ASSERT_FALSE(sub2.empty());
    ASSERT_EQ(sub2.back(), 4);
    ASSERT_EQ(sub2.front(), 3);
}

Test(popping)
{
    BroadcastQueue<int> queue;

    auto sub1 = queue.subscribe();
    auto sub2 = queue.subscribe();

    queue.push(1);
    queue.push(2);
    queue.push(3);
    queue.push(4);

    ASSERT_EQ(sub1.front(), 1);
    ASSERT_EQ(sub1.back(), 4);
    sub1.pop();
    ASSERT_EQ(sub1.front(), 2);
    ASSERT_EQ(sub1.back(), 4);
    sub1.pop();
    ASSERT_EQ(sub1.front(), 3);
    ASSERT_EQ(sub1.back(), 4);
    sub1.pop();
    ASSERT_EQ(sub1.front(), 4);
    ASSERT_EQ(sub1.back(), 4);
    sub1.pop();
    ASSERT_TRUE(sub1.empty());

    ASSERT_FALSE(sub2.empty());
    ASSERT_EQ(sub2.size(), 4);
    ASSERT_EQ(sub2.front(), 1);
    ASSERT_EQ(sub2.front(), 1);
    ASSERT_EQ(sub2.back(), 4);
    sub2.pop();
    ASSERT_EQ(sub2.size(), 3);
    ASSERT_EQ(sub2.front(), 2);
    ASSERT_EQ(sub2.back(), 4);
}

Test(clear)
{
    BroadcastQueue<int> queue;

    auto sub1 = queue.subscribe();
    queue.push(1);
    queue.push(2);
    queue.push(3);

    auto sub2 = queue.subscribe();
    queue.push(4);
    queue.push(5);
    queue.push(6);

    ASSERT_EQ(sub1.size(), 6);
    ASSERT_EQ(sub2.size(), 3);

    queue.clear();
    ASSERT_TRUE(sub1.empty());
    ASSERT_EQ(sub1.size(), 0);
    ASSERT_TRUE(sub2.empty());
    ASSERT_EQ(sub2.size(), 0);
}
