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

// Grpc
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <grpc/grpc.h>

// Proto
#include "signaling.grpc.pb.h"

// Graybat
#include <graybat/signaling/GrpcSignalingTypes.hpp>

namespace graybat {
namespace signaling {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

class GrpcSignalingClient {
  public:
    GrpcSignalingClient(const Uri& serverUri)
        : stub_(Signaling::NewStub(
              grpc::CreateChannel(serverUri, grpc::InsecureChannelCredentials())))
    {
    }

    bool RequestContext(const ContextRequest& request, ContextReply* reply);

    bool RequestVaddr(const VaddrRequest& request, VaddrReply* reply);

    bool LookupVaddr(const VaddrLookup& request, UriReply* reply);

    bool LeaveContext(const LeaveRequest& request, LeaveReply* reply);

  private:
    bool checkStatus(const Status& status);

  private:
    std::unique_ptr<Signaling::Stub> stub_;
};

inline bool GrpcSignalingClient::RequestContext(const ContextRequest& request, ContextReply* reply)
{
    ClientContext context;
    auto status = stub_->RequestContext(&context, request, reply);

    return checkStatus(status);
}

inline bool GrpcSignalingClient::RequestVaddr(const VaddrRequest& request, VaddrReply* reply)
{
    ClientContext context;
    auto status = stub_->RequestVaddr(&context, request, reply);

    return checkStatus(status);
}

inline bool GrpcSignalingClient::LookupVaddr(const VaddrLookup& request, UriReply* reply)
{
    ClientContext context;
    auto status = stub_->LookupVaddr(&context, request, reply);

    return checkStatus(status);
}

inline bool GrpcSignalingClient::LeaveContext(const LeaveRequest& request, LeaveReply* reply)
{
    ClientContext context;
    auto status = stub_->LeaveContext(&context, request, reply);

    return checkStatus(status);
}

inline bool GrpcSignalingClient::checkStatus(const Status& status)
{
    if (!status.ok()) {
        std::cout << "rpc failed: " << status.error_code() << " " << status.error_message()
                  << std::endl;
        return false;
    }
    return true;
}

} // namespace signaling
} // namespace graybat