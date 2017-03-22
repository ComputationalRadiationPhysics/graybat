
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

namespace graybat {
namespace threading {

class ThreadPoolBase {
  public:
    /** Pushes a task onto the execution queue of the threadpool.
  * The task will be executed on one of the threads in the threadpool.
  * If the threadpool is empty then the task will be executed on
  * the same thread as the caller.
  *
  * Example:
  *
  * * post([](){ std::cout << "Hello World" << std::endl;});
  *
  *@param task the task to be called
  */

    template <typename T_Task> void post(const T_Task& task) = delete;
};
}
}