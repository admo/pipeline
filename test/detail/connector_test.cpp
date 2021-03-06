/**
 * Boost.Pipeline
 *
 * Copyright 2014 Benedek Thaler
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See $$PIPELINE_WEBSITE$$ for documentation
 */

#include <type_traits>
#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include <boost/pipeline.hpp>
#include <boost/preprocessor/stringize.hpp>

#define BOOST_TEST_MODULE Connector
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;
using namespace boost::pipeline::detail;
using boost::pipeline::detail::connector;

//#define BOOST_PIPELINE_TEST_DEBUG 1
template <typename> std::string get_type_name();

struct unspec {};

// Available transformation types
// (specific to this test, long -> int only)
// std::function requires definition
int    oo(long) { return 0; } // 1-1
unspec on(long, queue_back<int>&) { return unspec(); } // 1-N
unspec nm(queue_front<long>&, queue_back<int>&) { return unspec(); } // N-M
int    no(queue_front<long>&) { return 0; } // N-1
void   co(long) { /* consume */ }
void   cm(queue_front<long>&) { /* consume */ }

// Possible segment types, specific to this test
template <typename T> struct dummy_plan_t
{
  typedef void root_type;
  typedef T value_type;
};

using dummy_plan = dummy_plan_t<long>;

typedef one_one_segment<dummy_plan, int> seg_oo;
typedef one_n_segment<dummy_plan, int, unspec> seg_on;
typedef n_m_segment<dummy_plan, int, unspec> seg_nm;
typedef n_one_segment<dummy_plan, int> seg_no;
typedef range_output_segment<dummy_plan, std::vector<long>> seg_ro;
typedef one_one_segment<dummy_plan, void> seg_co;
typedef n_one_segment<dummy_plan, void> seg_cm;

#if defined BOOST_PIPELINE_TEST_DEBUG
  #define TEST_CONNECTION(expr, expected) \
  { \
    auto actual = get_type_name<connector<dummy_plan, decltype(expr)>::type>(); \
    auto expStr = get_type_name<expected>(); \
    BOOST_CHECK_EQUAL(actual, expStr); \
  } \
  /**/
#else
  #define TEST_CONNECTION(expr, expected) \
    static_assert(  \
      std::is_same<connector<dummy_plan, decltype(expr)>::type, expected>::value,  \
      "Failed to connect: " BOOST_PP_STRINGIZE(expr)  \
      ", -DBOOST_PIPELINE_TEST_DEBUG to get more information." \
    );  \
    /**/
#endif

BOOST_AUTO_TEST_CASE(ConnectorFunctionPointer)
{
  TEST_CONNECTION(oo, seg_oo)
  TEST_CONNECTION(on, seg_on)
  TEST_CONNECTION(nm, seg_nm)
  TEST_CONNECTION(no, seg_no)
  TEST_CONNECTION(co, seg_co)
  TEST_CONNECTION(cm, seg_cm)
}

BOOST_AUTO_TEST_CASE(ConnectorFunction)
{
  auto f_oo = std::function<int(long)>(oo);
  auto f_on = std::function<unspec(long, queue_back<int>&)>(on);
  auto f_nm = std::function<unspec(queue_front<long>&, queue_back<int>&)>(nm);
  auto f_no = std::function<int(queue_front<long>&)>(no);
  auto f_co = std::function<void(long)>(co);
  auto f_cm = std::function<void(queue_front<long>&)>(cm);

  TEST_CONNECTION(f_oo, seg_oo)
  TEST_CONNECTION(f_on, seg_on)
  TEST_CONNECTION(f_nm, seg_nm)
  TEST_CONNECTION(f_no, seg_no)
  TEST_CONNECTION(f_co, seg_co)
  TEST_CONNECTION(f_cm, seg_cm)
}

BOOST_AUTO_TEST_CASE(ConnectorLambda)
{
  // non-mutable lambda
  auto l_oo = [](long) -> int { return 0; };
  auto l_on = [](long, queue_back<int>&) { return unspec(); };
  auto l_nm = [](queue_front<long>&, queue_back<int>&) { return unspec(); };
  auto l_no = [](queue_front<long>&) -> int { return 0; };
  auto l_co = [](long) -> void {};
  auto l_cm = [](queue_front<long>&) -> void {};

  TEST_CONNECTION(l_oo, seg_oo)
  TEST_CONNECTION(l_on, seg_on)
  TEST_CONNECTION(l_nm, seg_nm)
  TEST_CONNECTION(l_no, seg_no)
  TEST_CONNECTION(l_co, seg_co)
  TEST_CONNECTION(l_cm, seg_cm)

  // mutable lambda
  auto ml_oo = [](long) mutable -> int { return 0; };
  auto ml_on = [](long, queue_back<int>&) mutable { return unspec(); };
  auto ml_nm = [](queue_front<long>&, queue_back<int>&) mutable { return unspec(); };
  auto ml_no = [](queue_front<long>&) mutable -> int { return 0; };
  auto ml_co = [](long) mutable -> void {};
  auto ml_cm = [](queue_front<long>&) mutable -> void {};

  TEST_CONNECTION(ml_oo, seg_oo)
  TEST_CONNECTION(ml_on, seg_on)
  TEST_CONNECTION(ml_nm, seg_nm)
  TEST_CONNECTION(ml_no, seg_no)
  TEST_CONNECTION(ml_co, seg_co)
  TEST_CONNECTION(ml_cm, seg_cm)
}

// functors
struct oo_t { int    operator()(long) { return 0; } };
struct on_t { unspec operator()(long, queue_back<int>&) { return unspec(); } };
struct nm_t { unspec operator()(queue_front<long>&, queue_back<int>&) { return unspec(); } };
struct no_t { int    operator()(queue_front<long>&) { return 0; } };
struct co_t { void   operator()(long) {} };
struct cm_t { void   operator()(queue_front<long>&) {} };

BOOST_AUTO_TEST_CASE(ConnectorFunctor)
{
  oo_t t_oo;
  on_t t_on;
  nm_t t_nm;
  no_t t_no;
  co_t t_co;
  cm_t t_cm;

  TEST_CONNECTION(t_oo, seg_oo)
  TEST_CONNECTION(t_on, seg_on)
  TEST_CONNECTION(t_nm, seg_nm)
  TEST_CONNECTION(t_no, seg_no)
  TEST_CONNECTION(t_co, seg_co)
  TEST_CONNECTION(t_cm, seg_cm)
}

// Boundable transformations
int oo_b(char, long) { return 0; } // 1-1
int on_b(char, long, queue_back<int>&) { return 0; } // 1-N
int nm_b(char, queue_front<long>&, queue_back<int>&) { return 0; } // N-M
int no_b(char, queue_front<long>&) { return 0; } // N-1
void co_b(char, long) { /* consume */ }
void cm_b(char, queue_front<long>&) { /* consume */ }

// T-T boundables
void tt_on_b(char, long, queue_back<long>&) {} // 1-N
void tt_nm_b(char, queue_front<long>&, queue_back<long>&) {} // N-M

BOOST_AUTO_TEST_CASE(ConnectorBind)
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  typedef one_n_segment<dummy_plan, int, int> int_seg_on;
  typedef n_m_segment<dummy_plan, int, int> int_seg_nm;

  TEST_CONNECTION(std::bind(oo_b, 'a', _1), seg_oo)
  TEST_CONNECTION(std::bind(on_b, 'a', _1, _2), int_seg_on)
  TEST_CONNECTION(std::bind(nm_b, 'a', _1, _2), int_seg_nm)
  TEST_CONNECTION(std::bind(no_b, 'a', _1), seg_no)
  TEST_CONNECTION(std::bind(co_b, 'a', _1), seg_co)
  TEST_CONNECTION(std::bind(cm_b, 'a', _1), seg_cm)

  // T-T transformations are not required to
  // denote output queue type in return type
  typedef one_n_segment<dummy_plan, long, void> void_seg_on;
  typedef n_m_segment<dummy_plan, long, void> void_seg_nm;

  TEST_CONNECTION(std::bind(tt_on_b, 'a', _1, _2), void_seg_on)
  TEST_CONNECTION(std::bind(tt_nm_b, 'a', _1, _2), void_seg_nm)
}

BOOST_AUTO_TEST_CASE(ConnectorContainer)
{
  std::vector<long> container;
  TEST_CONNECTION(container, seg_ro)
}

#if defined BOOST_PIPELINE_TEST_DEBUG

#include <cxxabi.h>

template <typename T>
std::string get_type_name()
{
  int status = 0;
  auto name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);

  if (name)
  {
    std::string str(name);
    free(name);
    return str;
  }
  else
  {
    std::cerr << "Failed to demangle, status: " << status << std::endl;
    return "N/A";
  }
}

#endif
