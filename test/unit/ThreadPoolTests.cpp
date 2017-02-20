/**
 * Copyright 2017 Erik Zenker
 *
 * This file is part of Graybat.
 *
 * Graybat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graybat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Graybat.
 * If not, see <http://www.gnu.org/licenses/>.
 */

// Stl
#include <chrono>
#include <future>

// Boost
#include <boost/test/unit_test.hpp>

// Graybat
#include <graybat/threading/AsioThreadPool.hpp>

/*******************************************************************************
 * Fixtures
 *******************************************************************************/
/*******************************************************************************
 * Zero Thread Tests
 *******************************************************************************/
struct ZeroThreadPoolTests {

    ZeroThreadPoolTests()
        : timeout_(std::chrono::milliseconds(1000))
    {
    }

    graybat::threading::AsioThreadPool<0> threadPool_;
    std::chrono::milliseconds timeout_;
};

BOOST_FIXTURE_TEST_SUITE(zero_thread_pool, ZeroThreadPoolTests)

BOOST_AUTO_TEST_CASE(shouldExecuteTaskOnSameThread)
{
    std::promise<std::thread::id> threadId;
    threadPool_.post([&threadId]() { threadId.set_value(std::this_thread::get_id()); });

    BOOST_REQUIRE_EQUAL(threadId.get_future().get(), std::this_thread::get_id());
}
BOOST_AUTO_TEST_SUITE_END()

/*******************************************************************************
 * Single Thread Tests
 *******************************************************************************/
struct SingleThreadPoolTests {

    SingleThreadPoolTests()
        : timeout_(std::chrono::milliseconds(1000))
    {
    }

    graybat::threading::AsioThreadPool<1> threadPool_;
    std::chrono::milliseconds timeout_;
};

BOOST_FIXTURE_TEST_SUITE(single_thread_pool, SingleThreadPoolTests)

BOOST_AUTO_TEST_CASE(shouldSetPromise)
{
    std::promise<void> promise;
    threadPool_.post([&promise]() { promise.set_value(); });

    auto future = promise.get_future();
    BOOST_REQUIRE(future.wait_for(timeout_) == std::future_status::ready);
}

BOOST_AUTO_TEST_CASE(shouldExecuteOnDifferentThread)
{
    std::promise<std::thread::id> threadId;
    threadPool_.post([&threadId]() { threadId.set_value(std::this_thread::get_id()); });

    BOOST_REQUIRE_NE(threadId.get_future().get(), std::this_thread::get_id());
}

BOOST_AUTO_TEST_SUITE_END()
