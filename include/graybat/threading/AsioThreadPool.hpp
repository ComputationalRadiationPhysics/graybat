/**
 * Copyright 2016 Erik Zenker
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
#pragma once

// Stl
#include <functional>
#include <memory>
#include <thread>
#include <vector>

// Boost
#include <boost/asio/io_service.hpp>

// Graybat
#include <graybat/threading/ThreadPoolBase.hpp>

namespace graybat {
namespace threading {

template <unsigned nThreads> class AsioThreadPool : public ThreadPoolBase {
  public:
    AsioThreadPool()
        : work_(std::make_shared<boost::asio::io_service::work>(ioService_))
    {
        for (std::size_t i = 0; i < nThreads; ++i) {
            threads_.emplace_back([this]() { ioService_.run(); });
        }
    }

    ~AsioThreadPool()
    {
        work_.reset();
        ioService_.stop();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

  public: // ThreadPoolBase
    template <typename T_Task> void post(const T_Task& task);

  private:
    boost::asio::io_service ioService_;
    std::shared_ptr<boost::asio::io_service::work> work_;
    std::vector<std::thread> threads_;
};

template <unsigned nThreads>
template <typename T_Task>
void AsioThreadPool<nThreads>::post(const T_Task& task)
{
    ioService_.post(task);
}

template <> template <typename T_Task> void AsioThreadPool<0>::post(const T_Task& task)
{
    task();
}
}
}