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

// Stl
#include <map>
#include <mutex>

// Boost
#include <boost/core/ignore_unused.hpp>

// Grpc
#include <grpc++/grpc++.h>
#include "signaling.grpc.pb.h"

// Graybat
#include <graybat/signaling/GrpcSignalingTypes.hpp>


namespace graybat {

namespace signaling {

struct GrpcSignalingService final : public Signaling::Service {

    GrpcSignalingService()
        : maxContextId_(0)
        , maxInitialContextId_(0)
    {
    }

    // Signaling::Service
  public:
    grpc::Status RequestContext(
        grpc::ServerContext* context,
        const ContextRequest* contextRequst,
        ContextReply* contextReply) override;

    grpc::Status RequestVaddr(
        grpc::ServerContext* context,
        const VaddrRequest* request,
        VaddrReply* response) override;

    grpc::Status LookupVaddr(
        grpc::ServerContext* context,
        const VaddrLookup* request,
        UriReply* response) override;

    grpc::Status LeaveContext(
        grpc::ServerContext* context,
        const LeaveRequest* request,
        LeaveReply* response) override;

  private:
    std::mutex mtx_;
    std::map<ContextId, std::map<Vaddr, Uri>> phoneBook_;
    std::map<ContextId, std::map<Vaddr, Uri>> ctrlPhoneBook_;
    std::map<ContextId, Vaddr> maxVAddr_;
    std::map<ContextId, std::vector<Uri>> peers_;
    std::map<ContextName, ContextId> contextIds_;
    ContextId maxContextId_;
    ContextId maxInitialContextId_;
};

inline grpc::Status GrpcSignalingService::RequestContext(
    grpc::ServerContext*, const ContextRequest* request, ContextReply* response)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::string contextName = request->context_name();
    ContextId contextId = 0;

    if (contextIds_.find(contextName) != contextIds_.end()) {
        response->set_context_id(contextIds_.at(contextName));
        contextId = contextIds_.at(contextName);
    } else {
        contextId = maxContextId_;
        response->set_context_id(contextId);
        contextIds_[contextName] = contextId;
        maxContextId_++;
    }

    std::cout << "CONTEXT REQUEST [name:" << contextName << "]: " << contextId << std::endl;

    return grpc::Status::OK;
}

inline grpc::Status GrpcSignalingService::RequestVaddr(
    ::grpc::ServerContext*, const ::VaddrRequest* request, ::VaddrReply* response)
{
    std::lock_guard<std::mutex> lock(mtx_);
    ContextId contextId = request->context_id();
    Uri dataUri = request->data_uri();
    Uri ctrlUri = request->ctrl_uri();

    peers_[contextId].push_back(dataUri);
    Vaddr vAddr = peers_[contextId].size() - 1;

    phoneBook_[contextId][vAddr] = dataUri;
    ctrlPhoneBook_[contextId][vAddr] = ctrlUri;

    response->set_vaddr(vAddr);

    std::cout << "VADDR REQUEST [contextID:" << contextId << "][srcUri:" << dataUri << "]"
              << "[ctrlUri:" << ctrlUri << "]:" << vAddr << std::endl;

    return grpc::Status::OK;
}

inline grpc::Status GrpcSignalingService::LookupVaddr(
    ::grpc::ServerContext*, const ::VaddrLookup* request, ::UriReply* response)
{
    ContextId contextId = request->context_id();
    Vaddr vaddr = request->vaddr();

    if (phoneBook_[contextId].count(vaddr) == 0) {
        std::cout << "VADDR LOOKUP [contextID:" << contextId << "][remoteVAddr:" << vaddr << "]: "
                  << " RETRY " << std::endl;
    } else {
        response->set_data_uri(phoneBook_[contextId][vaddr]);
        response->set_ctrl_uri(ctrlPhoneBook_[contextId][vaddr]);

        std::cout << "VADDR LOOKUP [contextID:" << contextId << "][remoteVAddr:" << vaddr
                  << "]: " << phoneBook_[contextId][vaddr] << " " << ctrlPhoneBook_[contextId][vaddr]
                  << std::endl;
    }

    return grpc::Status::OK;
}

inline grpc::Status GrpcSignalingService::LeaveContext(
    grpc::ServerContext*, const LeaveRequest* request, LeaveReply*)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::string contextName = request->context_name();

    auto it = contextIds_.find(contextName);


    if (it != contextIds_.end()) {
        contextIds_.erase(it);
    }

    std::cout << "LEAVE CONTEXT [contextName:" << contextName << "]" << std::endl;

    return grpc::Status::OK;
}

} // namespace signaling
} // namespace grpc