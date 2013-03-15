//
// msgpack::rpc::session_pool - MessagePack-RPC for C++
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
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
#include "session_pool_impl.h"
#include "session_impl.h"
#include "future_impl.h"
#include "exception_impl.h"
#include "transport/tcp.h"
#include "cclog/cclog.h"

namespace msgpack {
namespace rpc {


static const unsigned int SESSION_POOL_TIME_LIMIT = 60;  // TODO


MP_UTIL_DEF(session_pool) {
	void start_timeout();
	static bool step_timeout(weak_session_pool wsp);
};

void MP_UTIL_IMPL(session_pool)::start_timeout()
{
	get_loop()->add_timer(1.0, 1.0, mp::bind(
				&MP_UTIL_IMPL(session_pool)::step_timeout,
				weak_session_pool(m_pimpl)
				));
}

bool MP_UTIL_IMPL(session_pool)::step_timeout(weak_session_pool wsp)
{
	shared_session_pool sp = wsp.lock();
	if(!sp) {
		return false;
	}
	sp->step_timeout();
	return true;
}

session_pool_impl::session_pool_impl(const builder& b, loop lo) :
	m_loop(lo),
	m_builder(b.copy()),
	m_pool_time_limit_sec( SESSION_POOL_TIME_LIMIT ),
	m_pool_size_limit(0) {
}

session_pool_impl::~session_pool_impl() { }

session session_pool_impl::get_session(const address& addr)
{
	table_ref ref(m_table);

	table_t::iterator found = ref->find(addr);
	if(found != ref->end()) {
		found->second.ttl = m_pool_time_limit_sec;
		return session(found->second.session);
	}

	if ( m_pool_size_limit > 0 && ref->size() >= m_pool_size_limit )
		throw too_many_session_error();

	shared_session s(session_impl::create(*m_builder, addr, m_loop));
	ref->insert( table_t::value_type(addr, entry_t(s, m_pool_time_limit_sec)) );

	return session(s);
}

void session_pool_impl::step_timeout()
{
	std::vector<shared_future> timedout;

	table_ref ref(m_table);
	for(table_t::iterator it(ref->begin()); it != ref->end(); ) {
		entry_t& e = it->second;
		if(e.session.unique()) {
			// There are no contexts that references the session.
			if(e.ttl <= 0) {
				// If e.session.unique() is true, m_pimpl->m_reqtable is empty
				// because it contains futures that references a session.
				ref->erase(it++);
				continue;
			}
			--e.ttl;
		}
		e.session->step_timeout(&timedout);
		++it;
	}
	ref.reset();

	if(!timedout.empty()) {
		for(std::vector<shared_future>::iterator it(timedout.begin()),
				it_end(timedout.end()); it != it_end; ++it) {
			shared_future& f = *it;
			f->set_result(object(), TIMEOUT_ERROR, auto_zone());
		}
	}
}

void session_pool_impl::set_pool_time_limit(int limit_sec) {
	table_ref ref(m_table);
	m_pool_time_limit_sec = limit_sec;
}

void session_pool_impl::set_pool_size_limit(size_t limit) {
	table_ref ref(m_table);
	m_pool_size_limit = limit;
}

size_t session_pool_impl::get_pool_size() {
	table_ref ref(m_table);
	return ref->size();
}

session_pool::session_pool(loop lo) :
	m_pimpl(new session_pool_impl(tcp_builder(), lo))
{
	MP_UTIL.start_timeout();
}

session_pool::session_pool(const builder& b, loop lo) :
	m_pimpl(new session_pool_impl(b, lo))
{
	MP_UTIL.start_timeout();
}

session_pool::session_pool(shared_session_pool pimpl) :
	m_pimpl(pimpl)
{
	MP_UTIL.start_timeout();
}

session_pool::~session_pool() { }

session session_pool::get_session(const address& addr)
	{ return m_pimpl->get_session(addr); }

const loop& session_pool::get_loop() const
	{ return const_cast<const session_pool_impl*>(m_pimpl.get())->get_loop(); }

loop session_pool::get_loop()
	{ return m_pimpl->get_loop(); }

void session_pool::set_pool_time_limit(int limit_sec)
	{ m_pimpl->set_pool_time_limit(limit_sec); }

int session_pool::get_pool_time_limit()
	{ return m_pimpl->get_pool_time_limit(); }

void session_pool::set_pool_size_limit(size_t limit)
	{ m_pimpl->set_pool_size_limit(limit); }

size_t session_pool::get_pool_size_limit()
	{ return m_pimpl->get_pool_size_limit(); }

size_t session_pool::get_pool_size()
	{ return m_pimpl->get_pool_size(); }

}  // namespace rpc
}  // namespace msgpack

