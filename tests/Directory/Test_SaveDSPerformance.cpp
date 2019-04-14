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
#include "libDirectoryService/DSComposition.h"
#include "libNetwork/ShardStruct.h"
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
#define BLOCK_NUM 1
#define EPOCH_NUM 1
#define DS_DIFF 1
#define SHARD_DIFF 1
#define GAS_PRICE 1
#define NUM_OF_FINAL_BLOCK 100
#define FINALBLOCK_REWARD -1

using namespace std;

BOOST_AUTO_TEST_SUITE(savedsperformance)

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

// Test that no previous performance data is carried over.

BOOST_FIXTURE_TEST_CASE(test_CleanSave, F) {
  INIT_STDOUT_LOGGER();

  // Create an empty coinbase rewards.
  std::map<uint64_t, std::map<int32_t, std::vector<PubKey>>> coinbaseRewardees;

  // Create some previous data in the member performance.
  std::map<PubKey, uint32_t> dsMemberPerformance;
  for (const auto& member : dsComm) {
    dsMemberPerformance[member.first] = 10;
  }

  InternalSaveDSPerformance(coinbaseRewardees, dsMemberPerformance, dsComm,
                            EPOCH_NUM, NUM_OF_FINAL_BLOCK, FINALBLOCK_REWARD);

  // Check the size.
  BOOST_CHECK_MESSAGE(dsMemberPerformance.size() == COMMITTEE_SIZE,
                      "Expected DS Performance size wrong. Actual: "
                          << dsMemberPerformance.size()
                          << ". Expected: " << COMMITTEE_SIZE);

  // Check the result.
  for (const auto& member : dsComm) {
    BOOST_CHECK_MESSAGE(dsMemberPerformance.at(member.first) != 0,
                        "Pub Key " << member.first
                                   << " is not cleared. Expected: 0. Result: "
                                   << dsMemberPerformance.at(member.first));
  }
}

// Test the legitimate case.

BOOST_FIXTURE_TEST_CASE(test_LegitimateCase, F) {
  INIT_STDOUT_LOGGER();

  // Create the winners.
  std::map<PubKey, Peer> winners;
  for (int i = 0; i < NUM_OF_ELECTED; ++i) {
    PairOfKey candidateKeyPair = Schnorr::GetInstance().GenKeyPair();
    PubKey candidatePubKey = candidateKeyPair.second;
    Peer candidatePeer = Peer(LOCALHOST, BASE_PORT + COMMITTEE_SIZE + i);
    winners[candidatePubKey] = candidatePeer;
  }

  // Create the nodes to be removed. This should be empty for this test case.
  std::vector<PubKey> removeDSNodePubkeys;

  // Construct the fake DS Block.
  PairOfKey leaderKeyPair = Schnorr::GetInstance().GenKeyPair();
  PubKey leaderPubKey = leaderKeyPair.second;
  DSBlockHeader header(DS_DIFF, SHARD_DIFF, leaderPubKey, BLOCK_NUM, EPOCH_NUM,
                       GAS_PRICE, SWInfo(), winners, removeDSNodePubkeys,
                       DSBlockHashSet());
  DSBlock block(header, CoSignatures());

  // Build the expected composition.
  DequeOfNode expectedDSComm;
  // Put the winners in front.
  for (const auto& i : winners) {
    // New additions are always placed at the beginning.
    expectedDSComm.emplace_front(i);
  }
  // Shift the existing members less NUM_OF_ELECTED to the back of the deque.
  for (int i = 0; i < (COMMITTEE_SIZE - NUM_OF_ELECTED); ++i) {
    expectedDSComm.emplace_back(dsComm.at(i));
  }

  // Check expected commmitee size.
  BOOST_CHECK_MESSAGE(expectedDSComm.size() == COMMITTEE_SIZE,
                      "Expected DS Committee size wrong. Actual: "
                          << expectedDSComm.size()
                          << ". Expected: " << COMMITTEE_SIZE);

  // Update the DS Composition.
  InternalUpdateDSCommitteeComposition(selfPubKey, dsComm, block);

  // Check updated commmitee size.
  BOOST_CHECK_MESSAGE(dsComm.size() == COMMITTEE_SIZE,
                      "Updated DS Committee size wrong. Actual: "
                          << dsComm.size() << ". Expected: " << COMMITTEE_SIZE);

  // Check the result.
  for (int i = 0; i < COMMITTEE_SIZE; ++i) {
    // Compare the public keys.
    PubKey actual = dsComm.at(i).first;
    PubKey expected = expectedDSComm.at(i).first;
    BOOST_CHECK_MESSAGE(
        actual == expected,
        "Index: " << i << ". Expected: " << expected << ". Result: " << actual);
  }
}

BOOST_AUTO_TEST_SUITE_END()
