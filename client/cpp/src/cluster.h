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

#include "endpoint.h"

#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>

namespace sopmq {
    namespace client {
        
        ///
        /// A cluster is a collection of endpoints that is managed to provide
        /// fault tolerance in the face of errors
        ///
        class cluster : public boost::noncopyable
        {
        public:
            typedef std::shared_ptr<cluster> ptr;
            
        public:
            template <typename ColType>
            cluster(ColType epCol)
            {
                for (auto ep : epCol)
                {
                    _endpoints.push_back(ep);
                }
                
                shuffle_endpoints();
            }
            
            virtual ~cluster();
            
        private:
            ///
            /// Shuffles the endpoints we have in our collection
            ///
            void shuffle_endpoints();
            
            std::vector<shared::net::endpoint> _endpoints;
        };
        
    }
}