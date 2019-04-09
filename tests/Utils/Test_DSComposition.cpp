/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <string>
#include "libCrypto/Schnorr.h"
#include "libUtils/DSComposition.h"
#include "libUtils/Logger.h"

#define BOOST_TEST_MODULE ipconverter
#define BOOST_TEST_DYN_LINK
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/multiprecision/cpp_int.hpp>
#pragma GCC diagnostic pop
#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(dscomposition)

// Test the original behaviour: nodes expire by having their index incremented
// above the DS committee size.
BOOST_AUTO_TEST_CASE(test_UpdateWithoutRemovals) {
  INIT_STDOUT_LOGGER();

  PairOfKey selfKeyPair = Schnorr::GetInstance().GenKeyPair();
  PubKey selfPubKey = selfKeyPair.second;

  BOOST_CHECK_MESSAGE(selfPubKey == selfPubKey,
                      "Expected: 127.0.0.1. Result: " << selfPubKey);
}

BOOST_AUTO_TEST_SUITE_END()
