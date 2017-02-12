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
#include <string>

// Boost
#include <boost/test/unit_test.hpp>

// Graybat
#include <graybat/signaling/GrpcSignalingClient.hpp>
#include <graybat/signaling/GrpcSignalingService.hpp>


struct GrpcTests {

    GrpcTests()
            : signalingClient_("localhost:5000"),
              contextName_("context_grpc_test")
    {
    }

    ~GrpcTests(){
        leaveContext();
    }

    void initContext(){
        {
            ContextRequest request;
            ContextReply reply;

            request.set_context_name(contextName_);
            signalingClient_.RequestContext(request, &reply);
            contextId_ = reply.context_id();
        }

        {
            VaddrRequest request;
            VaddrReply reply;

            request.set_context_id(contextId_);
            request.set_data_uri("localhost:5001");
            request.set_ctrl_uri("localhost:5002");

            signalingClient_.RequestVaddr(request, &reply);
            vaddr_ = reply.vaddr();
        }

    }

    void leaveContext(){
        LeaveRequest request;
        LeaveReply reply;
        request.set_context_name(contextName_);
        signalingClient_.LeaveContext(request, &reply);
    }

    graybat::signaling::GrpcSignalingClient signalingClient_;
    std::string contextName_;
    ContextId contextId_;
    Vaddr vaddr_;
};


/*******************************************************************************
 * Test Suites
 *******************************************************************************/
BOOST_FIXTURE_TEST_SUITE(grpc, GrpcTests)

/***************************************************************************
 * Test Cases
 ****************************************************************************/
BOOST_AUTO_TEST_CASE(shouldRequestContext)
{
    ContextRequest request;
    ContextReply reply;
    request.set_context_name(contextName_);

    signalingClient_.RequestContext(request, &reply);

    BOOST_REQUIRE_EQUAL(reply.context_id(), 0);

    signalingClient_.RequestContext(request, &reply);

    BOOST_REQUIRE_EQUAL(reply.context_id(), 1);
}

BOOST_AUTO_TEST_CASE(shouldRequestVaddr)
{
    VaddrRequest request;
    VaddrReply reply;

    request.set_context_id(0);
    request.set_data_uri("localhost:5001");
    request.set_ctrl_uri("localhost:5002");

    signalingClient_.RequestVaddr(request, &reply);

    BOOST_REQUIRE_EQUAL(reply.vaddr(), 0);
}

BOOST_AUTO_TEST_CASE(shouldLookupVaddr)
{
    initContext();

    VaddrLookup request;
    UriReply reply;

    request.set_context_id(0);
    request.set_vaddr(0);

    signalingClient_.LookupVaddr(request, &reply);

    BOOST_REQUIRE_EQUAL(reply.data_uri(), "localhost:5001");
    BOOST_REQUIRE_EQUAL(reply.ctrl_uri(), "localhost:5002");
}

BOOST_AUTO_TEST_CASE(shouldReplyWithEmpyUriRpelyWhenVaddrNotExists)
{
    initContext();
    Vaddr notExistingVAddr = 1;

    VaddrLookup request;
    UriReply reply;

    request.set_context_id(0);
    request.set_vaddr(notExistingVAddr);

    signalingClient_.LookupVaddr(request, &reply);

    BOOST_REQUIRE_EQUAL(reply.data_uri(), "");
    BOOST_REQUIRE_EQUAL(reply.ctrl_uri(), "");

}

BOOST_AUTO_TEST_CASE(shouldLeaveContext)
{
    initContext();

    LeaveRequest request;
    LeaveReply reply;

    request.set_context_name(contextName_);

    signalingClient_.LeaveContext(request, &reply);
}

BOOST_AUTO_TEST_SUITE_END()
