//
// msgpack::rpc::exception - MessagePack-RPC for C++
//
// Copyright (C) 2010 FURUHASHI Sadayuki
// Copyright (C) 2013 Preferred Networks and Nippon Telegraph and Telephone Corporation.
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#include "exception_impl.h"
#include "protocol.h"
#include <sstream>
#include <string.h>

namespace msgpack {
namespace rpc {


static const char* TIMEOUT_ERROR_PTR = "request timed out";
static const char* CONNECT_ERROR_PTR = "connect failed";
static const char* REQUEST_CANCELLED_PTR = "request cancelled";
static const char* CONNECTION_CLOSED_ERROR_PTR = "connection closed";

const msgpack::object TIMEOUT_ERROR( msgpack::type::raw_ref(
			TIMEOUT_ERROR_PTR, strlen(TIMEOUT_ERROR_PTR)
			) );

const msgpack::object CONNECT_ERROR( msgpack::type::raw_ref(
			CONNECT_ERROR_PTR, strlen(CONNECT_ERROR_PTR)
			) );

const msgpack::object REQUEST_CANCELLED( msgpack::type::raw_ref(
                        REQUEST_CANCELLED_PTR, strlen(REQUEST_CANCELLED_PTR)
			) );

const msgpack::object CONNECTION_CLOSED_ERROR( msgpack::type::raw_ref(
                        CONNECTION_CLOSED_ERROR_PTR, strlen(CONNECTION_CLOSED_ERROR_PTR)
			) );

void throw_exception(future_impl* f)
{
	object err = f->error();

        // NOTE:
        //   TIMEOUT_ERROR, CONNECT_ERROR, REQUEST_CANCELLED,
        //   CONNECTION_CLOSED_ERROR and NEGATIVE_INTEGER are
        //   caused by *local* errors. Otherwise, caused by
        //   *remote* errors or delivered errors from remote.

	if(err.type == msgpack::type::BIN &&
			err.via.bin.ptr == TIMEOUT_ERROR_PTR) {
		throw timeout_error();

	} else if(err.type == msgpack::type::BIN &&
			err.via.bin.ptr == CONNECT_ERROR_PTR) {
		throw connect_error();

        } else if(err.type == msgpack::type::BIN &&
			err.via.bin.ptr == REQUEST_CANCELLED_PTR) {
		throw request_cancelled();

        } else if(err.type == msgpack::type::BIN &&
			err.via.bin.ptr == CONNECTION_CLOSED_ERROR_PTR) {
		throw connection_closed_error();

        } else if(err.type == msgpack::type::NEGATIVE_INTEGER ) {
                int system_errno = -err.via.i64;
                throw system_error( mp::system_error::errno_string(system_errno) );

                // NOTE: Local POSIX errno ( system-error ) carried as negative 
                //       integer. This format isn't so smart...

	} else if(err.type == msgpack::type::POSITIVE_INTEGER &&
			err.via.u64 == NO_METHOD_ERROR) {
		throw no_method_error();

	} else if(err.type == msgpack::type::POSITIVE_INTEGER &&
			err.via.u64 == ARGUMENT_ERROR) {
		throw argument_error();

        } else if ( err.type == msgpack::type::BIN ) {
          // maybe simple error message
          
          std::ostringstream os;
          os.write(err.via.bin.ptr, err.via.bin.size);
          throw remote_error(os.str(), future(f->shared_from_this()));

	} else {
		std::ostringstream os;
		os << "remote error: ";
		os << err;
		throw remote_error(os.str(), future(f->shared_from_this()));
	}
}


}  // namespace rpc
}  // namespace msgpack

