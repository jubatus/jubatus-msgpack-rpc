// Copyright (C) 2013 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.

#include "echo_server.h"
#include <jubatus/msgpack/rpc/server.h>
#include <jubatus/msgpack/rpc/session_pool.h>
#include <cclog/cclog.h>
#include <cclog/cclog_tty.h>

static void test_basic_features() {
  // create session pool
  rpc::session_pool sp;

  // get session
  rpc::session s = sp.get_session("127.0.0.1", 18811);

  // async call
  rpc::future fs[10];

  for(int i=0; i < 10; ++i) {
    fs[i] = s.call("add", 1, 2);
  }

  for(int i=0; i < 10; ++i) {
    int ret = fs[i].get<int>();
    std::cout << "async call: add(1, 2) = " << ret << std::endl;
  }
}

static void run_loop_given_secs(rpc::session_pool &sp, int secs) {
  msgpack::rpc::loop lo = sp.get_loop();
  time_t dead_line = time(NULL) + secs;
  while( time(NULL) <= dead_line ) {
    lo->run_once();
  }
}

static void test_pool_expire() {
  // create session pool
  int expire_time = 5;

  rpc::session_pool sp;
  sp.set_pool_time_limit(expire_time);

  // get session
  // NOTE: To free session, delete rpc::session s
  {
    rpc::session s = sp.get_session("127.0.0.1", 18811);

    // async call
    rpc::future fs = s.call("add", 1, 2);
    (void)fs.get<int>();
  }

  // check: session pool size
  int after_one_call = sp.get_pool_size();

  // NOTE: To expire session pool, we need run event loop
  run_loop_given_secs(sp, expire_time+1);

  int after_expire = sp.get_pool_size();
  if ( after_one_call != 1 ) throw std::runtime_error( "session_pool size != 1"  );
  if ( after_expire != 0 ) throw std::runtime_error( "session_pool size != 0"  );
  std::cout << "async call: session-pool expired by " << expire_time << std::endl;
}

static void test_pool_size_limit() {
  // create session pool and set size
  int limit_size = 2;
  int expire_time = 5;

  rpc::session_pool sp;
  sp.set_pool_size_limit(limit_size);
  sp.set_pool_time_limit(expire_time);
  
  // get session
  int port = 18811;
  rpc::session s0 = sp.get_session("127.0.0.1", port+0);
  std::cout << "ok: get_session(" << port+0 << ")" << std::endl;
  {
    rpc::session s1 = sp.get_session("127.0.0.1", port+1);
    std::cout << "ok: get_session(" << port+1 << ")" << std::endl;
    try {
      rpc::session s2 = sp.get_session("127.0.0.1", port+2);
      throw std::runtime_error( "async call: couldn't limit session-pool size" );

    } catch ( msgpack::rpc::too_many_session_error &e ) {
      std::cout << "ok: " << e.what() << std::endl;
      std::cout << "async call: session-pool size limit by 2" << std::endl;
    }
  }

  // wait expire 2nd session
  run_loop_given_secs( sp, expire_time+1 );

  // get session
  rpc::session s2 = sp.get_session("127.0.0.1", port+2);
  std::cout << "ok: get_session(" << port+2 << ")" << std::endl;
}

int main(void)
{
  cclog::reset(new cclog_tty(cclog::TRACE, std::cout));
  signal(SIGPIPE, SIG_IGN);

  // run server
  rpc::server svr;
  std::auto_ptr<rpc::dispatcher> dp(new myecho);
  svr.serve(dp.get());

  svr.listen("0.0.0.0", 18811);
  svr.start(4);

  // test basic features...
  test_basic_features();

  // test session-pool expire...
  test_pool_expire();

  // run more servers
  rpc::server svr2;
  std::auto_ptr<rpc::dispatcher> dp2(new myecho);
  svr2.serve(dp2.get());
  svr2.listen("0.0.0.0", 18812);
  svr2.start(2);

  rpc::server svr3;
  std::auto_ptr<rpc::dispatcher> dp3(new myecho);
  svr3.serve(dp3.get());
  svr3.listen("0.0.0.0", 18813);
  svr3.start(2);
  
  // test session-pool size limit
  test_pool_size_limit();

  return 0;
}
