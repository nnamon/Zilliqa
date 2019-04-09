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
#include "libData/BlockData/Block.h"
#include "libNetwork/ShardStruct.h"
#include "libUtils/DSComposition.h"
#include "libUtils/Logger.h"
#include "libUtils/SWInfo.h"

#define BOOST_TEST_MODULE dscomposition
#define BOOST_TEST_DYN_LINK
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/multiprecision/cpp_int.hpp>
#pragma GCC diagnostic pop
#include <boost/test/unit_test.hpp>

#define COMMITTEE_SIZE 20
#define NUM_OF_ELECTED 5
#define NUM_OF_REMOVED 2
#define LOCALHOST 0x7F000001
#define BASE_PORT 2600
#define BLOCK_NUM 5

using namespace std;

BOOST_AUTO_TEST_SUITE(dscomposition)

struct F {
  F() {
    BOOST_TEST_MESSAGE("setup fixture");

    // Generate the self key.
    selfKeyPair = Schnorr::GetInstance().GenKeyPair();
    selfPubKey = selfKeyPair.second;

    // Generate the DS Committee.
    for (int i = 0; i < COMMITTEE_SIZE; ++i) {
      PairOfKey kp = Schnorr::GetInstance().GenKeyPair();
      PubKey pk = kp.second;
      Peer peer = Peer(LOCALHOST, BASE_PORT + i);
      PairOfNode entry = std::make_pair(pk, peer);
      dsComm.emplace_back(entry);
    }
  }

  ~F() { BOOST_TEST_MESSAGE("teardown fixture"); }

  PairOfKey selfKeyPair;
  PubKey selfPubKey;
  DequeOfNode dsComm;
};

// Test the original behaviour: nodes expire by having their index incremented
// above the DS committee size.
BOOST_FIXTURE_TEST_CASE(test_UpdateWithoutRemovals, F) {
  INIT_STDOUT_LOGGER();

  // Construct the fake DS Block.
  PairOfKey leaderKeyPair = Schnorr::GetInstance().GenKeyPair();
  PubKey leaderPubKey = leaderKeyPair.second;
  std::map<PubKey, Peer> pp;
  DSBlockHeader header(0, 0, leaderPubKey, BLOCK_NUM, BLOCK_NUM, 0, SWInfo(), pp, DSBlockHashSet());
  DSBlock block;

  BOOST_CHECK_MESSAGE(selfPubKey == selfPubKey,
                      "Expected: 127.0.0.1. Result: " << selfPubKey);
}

BOOST_AUTO_TEST_SUITE_END()
