/**
 * Boost.Pipeline
 *
 * Copyright 2014 Benedek Thaler
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See $PIPELINE_WEBSITE$ for documentation
 */

#include <vector>
#include <deque>
#include <functional>

#include <boost/pipeline.hpp>

#define BOOST_TEST_MODULE OpenSegment
#include <boost/test/unit_test.hpp>

using namespace boost::pipeline;

int iidentity(const int a)
{
  return a;
}

long lidentity(const long a)
{
  return a;
}

BOOST_AUTO_TEST_CASE(OpenSegmentCtor)
{
  auto os1 = make(iidentity);
  auto os2 = os1 | lidentity;

  (void) os2;
}

int not_equals_to(
  const int filter,
  const int item,
  queue_back<int>& out
)
{
  if (item != filter)
  {
    out.push(item);
  }

  return item;
}

int make_odd(const int item)
{
  if (item & 1)
  {
    return item;
  }

  return item + 1;
}

void if_multiple_of(
  const int multiple,
  const int divisor,
  queue_back<int>& out
)
{
  if (multiple % divisor == 0)
  {
    out.push(multiple);
  }
}

BOOST_AUTO_TEST_CASE(SegmentOpen)
{
  using std::placeholders::_1;
  using std::placeholders::_2;

  auto plan1 = make(std::bind(not_equals_to, 7, _1, _2));
  auto plan2 = make(make_odd) | std::bind(if_multiple_of, _1, 3, _2);

  auto plan3 = plan1 | plan2;

  std::vector<int> input{1, 2, 3, 4, 5, 6, 7, 8, 9};
  std::vector<int> output;

  auto segment = from(input);

  thread_pool pool{1};

  auto exec =
    (segment | plan1 | plan3 | output).run(pool);
  exec.wait();

  std::vector<int> expected{3,3,9,9};

  BOOST_CHECK(output == expected);
}

BOOST_AUTO_TEST_CASE(ConnectContainer)
{
  std::vector<int> input{1, 2, 3, 4, 5};
  std::vector<int> output;

  auto plan1 = make(iidentity);
  auto plan2 = plan1 | output;
  auto plan3 = from(input) | plan2;

  thread_pool pool{1};
  auto exec = plan3.run(pool);
  exec.wait();

  std::vector<int> expected = input;
  BOOST_CHECK(output == expected);
}
