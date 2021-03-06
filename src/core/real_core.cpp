#include <glog/logging.h>
#include <transport/listen_tcp_socket.h>
#include <transport/curl_pool.h>
#include <rules_loader.h>
#include <external/real_external.h>
#include <core/real_core_helper.h>
#include <core/real_core.h>
#include <stdlib.h>
#include <string.h>
#include <util/util.h>
#include <unistd.h>

real_core::real_core(const config & conf)
  :
    config_(conf),

    instrumentation_(conf),

    executor_pool_(instrumentation_, conf),

    main_loop_(make_main_async_loop()),

    scheduler_(new real_scheduler()),

    externals_(new real_external(conf, instrumentation_)),

    streams_(new streams(conf, instrumentation_)),

    pubsub_(new pub_sub()),

    index_(new real_index(*pubsub_, [=](e_t e) { streams_->push_event(e); },
                          conf.index_expire_interval, *scheduler_,
                          instrumentation_, detach_thread)),

    tcp_server_(init_tcp_server(conf, *main_loop_, *streams_,
                executor_pool_, instrumentation_)),

    udp_server_(init_udp_server(conf, streams_)),

    ws_server_(init_ws_server(conf, *main_loop_, *pubsub_, *index_))
{

  if (conf.enable_internal_metrics) {
    start_instrumentation(*scheduler_, instrumentation_, *streams_);
  }

}

void real_core::start() {

  LOG(INFO) << "Brace for impact, starting nuclear core.";

  signal(SIGPIPE, SIG_IGN);

  streams_->reload_rules();

  main_loop_->start();

  VLOG(3) << "Stopping services";

  scheduler_->stop();
  executor_pool_.stop();
  streams_->stop();
  tcp_server_->stop();
  udp_server_->stop();
  ws_server_->stop();
  externals_->stop();

  LOG(INFO) << "Screw you guys, I'm going home.";
}

void real_core::add_stream(std::shared_ptr<streams_t> stream) {

  if (stream) {

    sh_streams_.push_back(stream);

    streams_->add_stream(*stream);

  }

}

real_core::~real_core() {
  VLOG(3) << "~real_core";
}

void real_core::reload_rules() {
  VLOG(3) << "reload_rules()";
  streams_->reload_rules();
}

index_interface & real_core::idx() {
  return *index_;
}

scheduler_interface & real_core::sched() {
  return *scheduler_;
}

external_interface & real_core::externals() {
  return *externals_;
}

void start_core(int argc, char **argv) {

  int orig_argc = argc;
  char **orig_argv = copy_args(argc, argv);

  FLAGS_stderrthreshold = 0;
  FLAGS_logbuflevel = -1;

  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  auto conf = create_config();

  ld_environment(orig_argv, conf.rules_directory);

  free_args(orig_argc, orig_argv);

  log_config(conf);

  g_core = make_real_core(conf);

  g_core->start();

  g_core.reset();

  free_thread_ns();
}
