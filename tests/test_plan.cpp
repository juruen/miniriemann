#include <glog/logging.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "basic_test_case.h"
#include "query_grammar_test_case.h"
#include "mock_scheduler_test_case.h"
#include "streams_test_case.h"
#include "folds_test_case.h"
#include "rules_common_test_case.h"
#include "tcp_connection_test_case.h"
#include "ws_connection_test_case.h"
#include "riemann_tcp_connection_test_case.h"
#include "pubsub_test_case.h"
#include "index_test_case.h"
#include <scheduler/mock_scheduler.h>
#include <core/mock_core.h>
#include "os/os_functions.h"
#include "os/mock_os_functions.h"

mock_os_functions mock_os;
os_functions g_os_functions(mock_os);

int main(int argc, char **argv)
{
  int ret;

  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  ::testing::InitGoogleMock(&argc, argv);

  auto m_core = std::make_shared<mock_core>();
  g_core = std::dynamic_pointer_cast<mock_core>(m_core);

  ret = RUN_ALL_TESTS();

  return ret;
}
