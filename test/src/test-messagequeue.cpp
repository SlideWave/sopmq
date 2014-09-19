/*
 * SOPMQ - Scalable optionally persistent message queue
 * Copyright 2014 InWorldz, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"

#include "message_queue.h"
#include "vector_clock.h"
#include "node_clock.h"
#include "util.h"

#include <boost/uuid/uuid.hpp>

using namespace sopmq::node;



TEST(MessageQueueTest, MessageQueueBasicOrdering)
{
    message_queue3 mq("abcde");
    
    node_clock a1 = {1, 1, 1};
    node_clock b1 = {2, 1, 0};
    node_clock c1 = {3, 1, 1};

    vector_clock3 m1clock;
    m1clock.set(0, a1);
    m1clock.set(1, b1);
    m1clock.set(2, c1);
    
    auto m1id = sopmq::shared::util::random_uuid();
    std::string m1content("message1");
    mq.enqueue(m1id, &m1content, 5);
    mq.stamp(m1id, m1clock);
    
    
    
    node_clock a3 = {1, 1, 2};
    node_clock b3 = {2, 1, 3};
    node_clock c3 = {3, 1, 2};
    
    vector_clock3 m3clock;
    m3clock.set(0, a3);
    m3clock.set(1, b3);
    m3clock.set(2, c3);
    
    auto m3id = sopmq::shared::util::random_uuid();
    std::string m3content("message3");
    mq.enqueue(m3id, &m3content, 5);
    mq.stamp(m3id, m3clock);
    
    
    
    node_clock a2 = {1, 1, 2};
    node_clock b2 = {2, 1, 2};
    node_clock c2 = {3, 1, 1};
    
    vector_clock3 m2clock;
    m2clock.set(0, a2);
    m2clock.set(1, b2);
    m2clock.set(2, c2);
    
    auto m2id = sopmq::shared::util::random_uuid();
    std::string m2content("message2");
    mq.enqueue(m2id, &m2content, 5);
    mq.stamp(m2id, m2clock);
    
    
    auto messages = mq.peekAll();
    ASSERT_EQ(m1id, messages[0]->id());
    ASSERT_EQ(m2id, messages[1]->id());
    ASSERT_EQ(m3id, messages[2]->id());
}