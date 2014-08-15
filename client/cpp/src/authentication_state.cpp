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

#include "authentication_state.h"
#include "session.h"
#include "logging.h"
#include "ChallengeResponseMessage.pb.h"
#include "AnswerChallengeMessage.pb.h"
#include "AuthAckMessage.pb.h"
#include "GetChallengeMessage.pb.h"
#include "util.h"
#include "messageutil.h"

#include <functional>
#include <boost/assert.hpp>
#include <cryptopp/sha.h>

using sopmq::message::message_dispatcher;
using namespace std::placeholders;
using sopmq::shared::util;
using sopmq::message::messageutil;

namespace sopmq {
    namespace client {
        namespace impl {
            
            authentication_state::authentication_state(cluster_connection::ptr conn, session::wptr session,
                                                       const std::string& username, const std::string& password,
                                                       std::function<void(bool)> authCallback)
            : _connection(conn), _session(session), _username(username), _password(password), _authCallback(authCallback)
            {
                _dispatcher =
                    std::make_shared<message_dispatcher>(std::bind(&authentication_state::on_unhandled_message,
                                                                   this, _1, _2));
                
                _connection->set_dispatcher(_dispatcher.get());
            }
            
            authentication_state::~authentication_state()
            {
                LOG_SRC(debug) << "~authentication_state()";
            }
            
            void authentication_state::state_entry()
            {
                LOG_SRC(debug) << "authentication_state::state_entry()";
                
                std::function<void(ChallengeResponseMessage_ptr)> func
                    = std::bind(&authentication_state::on_challenge_response, shared_from_this(), _1);
                
                _dispatcher->set_handler(func);
                
                //send a request to the server to get an auth challenge
                GetChallengeMessage_ptr gcm = std::make_shared<GetChallengeMessage>();
                gcm->set_type(GetChallengeMessage::CLIENT);
                gcm->set_allocated_identity(messageutil::build_id(_connection->get_next_id(), 0));
                
                _connection->send_message(message::MT_GET_CHALLENGE, gcm,
                                          std::bind(&authentication_state::on_message_sent, shared_from_this(), _1));
                
                _connection->get_next_message(std::bind(&authentication_state::on_message_received, shared_from_this(), _1));
            }
            
            void authentication_state::on_message_sent(const net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    //auth failed
                    LOG_SRC(error) << _connection->network_endpoint()
                        << " network error during session authorization: "
                        << result.get_error().what();
                    
                    _authCallback(false);
                }
            }
            
            void authentication_state::on_message_received(const net::network_operation_result& result)
            {
                if (!result.was_successful())
                {
                    //auth failed
                    LOG_SRC(error) << _connection->network_endpoint()
                        << " network error during session authorization: "
                        << result.get_error().what();
                    
                    _authCallback(false);
                }
            }
            
            void authentication_state::on_unhandled_message(Message_ptr message, const std::string& typeName)
            {
                LOG_SRC(error) << _connection->network_endpoint()
                    << " protocol violation: unexpected message: "
                    << typeName;
                
                if (auto session = _session.lock())
                {
                    session->protocol_violation();
                }
            }
            
            void authentication_state::on_challenge_response(ChallengeResponseMessage_ptr response)
            {
                BOOST_ASSERT(response->has_challenge());
                
                const std::string& challenge = response->challenge();
                
                //respond to the challenge with:
                //sha256([username]) + " " + sha256(sha256([password])[challenge])
                unsigned char hashResult[CryptoPP::SHA256::DIGESTSIZE];
                
                CryptoPP::SHA256 sha;
                
                sha.CalculateDigest(&hashResult[0], (unsigned char*)_username.c_str(), _username.length());
                std::string result = util::hex_encode(hashResult, CryptoPP::SHA256::DIGESTSIZE);
                
                result += " ";
                
                std::string pwAndChallenge(_password);
                pwAndChallenge += challenge;
                
                sha.CalculateDigest(&hashResult[0], (unsigned char*)pwAndChallenge.c_str(), pwAndChallenge.length());
                result += util::hex_encode(hashResult, CryptoPP::SHA256::DIGESTSIZE);
                
                //clear the handler for the challenge response since we're not looking for that anymore
                _dispatcher->set_handler(std::function<void(ChallengeResponseMessage_ptr)>());

                //set the handler for the AuthAck
				std::function<void(AuthAckMessage_ptr)> func = std::bind(&authentication_state::on_auth_ack, shared_from_this(), _1);
                _dispatcher->set_handler(func);
                
                AnswerChallengeMessage_ptr acm = std::make_shared<AnswerChallengeMessage>();
                acm->set_allocated_identity(messageutil::build_id(_connection->get_next_id(), response->identity().id()));
                acm->set_response(result);
                _connection->send_message(message::MT_ANSWER_CHALLENGE, acm,
                                          std::bind(&authentication_state::on_message_sent, shared_from_this(), _1));
                
                _connection->get_next_message(std::bind(&authentication_state::on_message_received, shared_from_this(), _1));
            }
            
            void authentication_state::on_auth_ack(AuthAckMessage_ptr response)
            {
                if (response->has_authorized() && response->authorized())
                {
                    _authCallback(true);
                }
                else
                {
                    //auth failed
                    LOG_SRC(error) << _connection->network_endpoint() << " session authorization denied";
                    _authCallback(false);
                }
            }
            
        }
    }
}