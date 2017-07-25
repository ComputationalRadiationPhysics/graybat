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

#pragma once

// STL
#include <memory>

// BOOST
#include <boost/hana/concat.hpp>
#include <boost/hana/for_each.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/test/unit_test.hpp>

// GRAYBAT
#include <graybat/graybat.hpp>

namespace graybat {
namespace test {
namespace utils {

auto inline getCommunicationPolicies()
{
#ifdef graybat_ZMQ_CP_ENABLED
    using ZMQ = graybat::communicationPolicy::ZMQ;
    using ZMQConfig = ZMQ::Config;
    ZMQConfig zmqConfig = { "localhost:5000",
                            "tcp://127.0.0.1:5001",
                            static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
                            "context_cp_test" };
    auto zmq = boost::hana::make_tuple(std::make_shared<ZMQ>(zmqConfig));
#else
    auto zmq = boost::hana::make_tuple();
#endif

#ifdef graybat_BMPI_CP_ENABLED
    using BMPI = graybat::communicationPolicy::BMPI;
    using BMPIConfig = BMPI::Config;
    BMPIConfig bmpiConfig;
    auto bmpi = boost::hana::make_tuple(std::make_shared<BMPI>(bmpiConfig));
#else
    auto bmpi = boost::hana::make_tuple();
#endif
    return boost::hana::concat(zmq, bmpi);
}

auto inline getCages()
{
    using Serialization = graybat::serializationPolicy::ByteCast;
    using GP = graybat::graphPolicy::BGL<>;
#ifdef graybat_ZMQ_CP_ENABLED
    using ZMQ = graybat::communicationPolicy::ZMQ;
    using ZMQConfig = ZMQ::Config;
    using ZMQCage = graybat::Cage<ZMQ, GP, Serialization>;
    ZMQConfig zmqConfig = { "localhost:5000",
                            "tcp://127.0.0.1:5001",
                            static_cast<size_t>(std::stoi(std::getenv("OMPI_COMM_WORLD_SIZE"))),
                            "context_cp_test" };
    auto zmqCage = boost::hana::make_tuple(std::make_shared<ZMQCage>(zmqConfig));
#else
    auto zmqCage = boost::hana::make_tuple();
#endif

#ifdef graybat_BMPI_CP_ENABLED
    using BMPI = graybat::communicationPolicy::BMPI;
    using BMPIConfig = BMPI::Config;
    using BMPICage = graybat::Cage<BMPI, GP, Serialization>;
    BMPIConfig bmpiConfig;
    auto bmpiCage = boost::hana::make_tuple(std::make_shared<BMPICage>(bmpiConfig));
#else
    auto bmpiCage = boost::hana::make_tuple();
#endif

    return boost::hana::concat(zmqCage, bmpiCage);
}
}
}
}