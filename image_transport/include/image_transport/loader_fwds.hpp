// Copyright (c) 2009, Willow Garage, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Willow Garage nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef IMAGE_TRANSPORT__LOADER_FWDS_HPP_
#define IMAGE_TRANSPORT__LOADER_FWDS_HPP_

#include <memory>

// Forward-declare some classes most users shouldn't care about so that
// image_transport.hpp doesn't bring them in.

namespace pluginlib
{
template<class T>
class ClassLoader;
}  // namespace pluginlib

namespace image_transport
{
template<class NodeType>
class PublisherPlugin;
template<class NodeType>
class SubscriberPlugin;

template<class NodeType>
using PubLoader = pluginlib::ClassLoader<PublisherPlugin<NodeType>>;
template<class NodeType>
using PubLoaderPtr = std::shared_ptr<PubLoader<NodeType>>;

template<class NodeType>
using SubLoader = pluginlib::ClassLoader<SubscriberPlugin<NodeType>>;
template<class NodeType>
using SubLoaderPtr = std::shared_ptr<SubLoader<NodeType>>;
}  // namespace image_transport

#endif  // IMAGE_TRANSPORT__LOADER_FWDS_HPP_
