#ifndef CAVALIERI_CORE_REAL_CORE_H
#define CAVALIERI_CORE_REAL_CORE_H

#include <core/core.h>
#include <instrumentation/instrumentation.h>
#include <streams/stream_functions.h>
#include <async/real_async_loop.h>
#include <pub_sub/pub_sub.h>
#include <index/real_index.h>
#include <riemann_tcp_pool.h>
#include <riemann_udp_pool.h>
#include <websocket/websocket_pool.h>
#include <pool/executor_thread_pool.h>
#include <external/real_external.h>
#include <scheduler/real_scheduler.h>
#include <config/config.h>

class real_core : public core_interface {
public:

  real_core(const config &);
  void start();
  void add_stream(std::shared_ptr<streams_t> stream);
  void reload_rules();
  index_interface & idx();
  scheduler_interface & sched();
  external_interface & externals();
  ~real_core();

private:
  config config_;

  instrumentation::instrumentation instrumentation_;

  executor_thread_pool executor_pool_;

  std::unique_ptr<main_async_loop_interface> main_loop_;
  std::unique_ptr<real_scheduler> scheduler_;
  std::unique_ptr<real_external> externals_;
  std::shared_ptr<streams> streams_;
  std::unique_ptr<pub_sub> pubsub_;
  std::unique_ptr<real_index> index_;
  std::unique_ptr<riemann_tcp_pool> tcp_server_;
  std::unique_ptr<riemann_udp_pool> udp_server_;
  std::unique_ptr<websocket_pool> ws_server_;

  std::vector<std::shared_ptr<streams_t>> sh_streams_;

};

#endif
