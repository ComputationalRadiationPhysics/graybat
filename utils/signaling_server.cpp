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

// Stl
#include <iostream> /* std::cout, std::endl */
#include <map>      /* std::map */
#include <string>   /* std::string */
#include <sstream>  /* std::stringstream */
#include <memory>   /* std::make_unique */

// Boost
#include <boost/program_options.hpp>

// Grpc
#include <grpc++/grpc++.h>

// Graybat
#include <graybat/signaling/GrpcSignalingService.hpp>
#include <graybat/signaling/GrpcSignalingTypes.hpp>


int main(const int argc, char **argv){
    /***************************************************************************
     * Parse Commandline
     **************************************************************************/
    namespace po = boost::program_options;
    po::options_description options( "Grpc Signaling Server Options" );

    options.add_options()
            ("port,p",
             po::value<unsigned>()->default_value(5000),
             "Port to listen for signaling requests")
            ("ip",
             po::value<std::string>()->default_value("localhost"),
             "IP to listen for signaling requests. Either ip or interface can be specified. (Example: 127.0.0.1)")
            ("help,h",
             "Print this help message and exit");


    po::variables_map vm;
    po::store(po::parse_command_line( argc, argv, options ), vm);

    if(vm.count("help")){
        std::cout << "Usage: " << argv[0] << " [options] " << std::endl;
        std::cout << options << std::endl;
        exit(0);
    }

    std::string masterUri;
    if(vm.count("ip")) {
        masterUri =  vm["ip"].as<std::string>() + ":" + std::to_string(vm["port"].as<unsigned>());
    }

    /***************************************************************************
     * Start signaling
     **************************************************************************/
    std::cout << "Start grpc signaling server" << std::endl;

    graybat::signaling::GrpcSignalingService service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(masterUri, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::cout << "Start listening on " << masterUri << std::endl;
    server->Wait();
}
