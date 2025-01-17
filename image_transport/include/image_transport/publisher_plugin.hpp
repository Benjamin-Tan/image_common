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

#ifndef IMAGE_TRANSPORT__PUBLISHER_PLUGIN_HPP_
#define IMAGE_TRANSPORT__PUBLISHER_PLUGIN_HPP_

#include <stdexcept>
#include <string>
#include <vector>
#include <memory>

#include "rclcpp/node.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "sensor_msgs/msg/image.hpp"

#include "image_transport/single_subscriber_publisher.hpp"
#include "image_transport/visibility_control.hpp"

namespace image_transport
{

/**
 * \brief Base class for plugins to Publisher.
 */
template<class NodeType = rclcpp::Node>
class PublisherPlugin
{
public:
  PublisherPlugin() = default;
  PublisherPlugin(const PublisherPlugin &) = delete;
  PublisherPlugin & operator=(const PublisherPlugin &) = delete;

  virtual ~PublisherPlugin() {}

  /**
   * \brief Get a string identifier for the transport provided by
   * this plugin.
   */
  virtual std::string getTransportName() const = 0;

  /**
   * \brief Check whether this plugin supports publishing using UniquePtr.
   */
  virtual bool supportsUniquePtrPub() const
  {
    return false;
  }

  /**
   * \brief Advertise a topic, simple version.
   */
  void advertise(
    NodeType * nh,
    const std::string & base_topic,
    rmw_qos_profile_t custom_qos = rmw_qos_profile_default,
    rclcpp::PublisherOptions options = rclcpp::PublisherOptions())
  {
    advertiseImpl(std::shared_ptr<NodeType>(nh), base_topic, custom_qos, options);
  }

  void advertise(
    std::shared_ptr<NodeType> nh,
    const std::string & base_topic,
    rmw_qos_profile_t custom_qos = rmw_qos_profile_default,
    rclcpp::PublisherOptions options = rclcpp::PublisherOptions())
  {
    advertiseImpl(nh, base_topic, custom_qos, options);
  }

  /**
   * \brief Returns the number of subscribers that are currently connected to
   * this PublisherPlugin.
   */
  virtual size_t getNumSubscribers() const = 0;

  /**
   * \brief Returns the communication topic that this PublisherPlugin will publish on.
   */
  virtual std::string getTopic() const = 0;

  /**
   * \brief Publish an image using the transport associated with this PublisherPlugin.
   */
  virtual void publish(const sensor_msgs::msg::Image & message) const = 0;

  /**
   * \brief Publish an image using the transport associated with this PublisherPlugin.
   */
  virtual void publishPtr(const sensor_msgs::msg::Image::ConstSharedPtr & message) const
  {
    publish(*message);
  }

  /**
   * \brief Publish an image using the transport associated with this PublisherPlugin.
   * This version of the function can be used to optimize cases where the Plugin can
   * avoid doing copies of the data when having the ownership to the image message.
   * Plugins that can take advantage of message ownership should overwrite this method
   * along with supportsUniquePtrPub().
   */
  virtual void publishUniquePtr(sensor_msgs::msg::Image::UniquePtr /*message*/) const
  {
    throw std::logic_error("publishUniquePtr() is not implemented.");
  }

  /**
   * \brief Publish an image using the transport associated with this PublisherPlugin.
   * This version of the function can be used to optimize cases where you don't want to
   * fill a ROS message first to avoid useless copies.
   * @param message an image message to use information from (but not data)
   * @param data a pointer to the image data to use to fill the Image message
   */
  virtual void publishData(const sensor_msgs::msg::Image & message, const uint8_t * data) const
  {
    sensor_msgs::msg::Image msg;
    msg.header = message.header;
    msg.height = message.height;
    msg.width = message.width;
    msg.encoding = message.encoding;
    msg.is_bigendian = message.is_bigendian;
    msg.step = message.step;
    msg.data = std::vector<uint8_t>(data, data + msg.step * msg.height);

    publish(msg);
  }

  /**
   * \brief Shutdown any advertisements associated with this PublisherPlugin.
   */
  virtual void shutdown() = 0;

  /**
   * \brief Return the lookup name of the PublisherPlugin associated with a specific
   * transport identifier.
   */
  static std::string getLookupName(const std::string & transport_name)
  {
    return "image_transport/" + transport_name + "_pub";
  }

protected:
  /**
   * \brief Advertise a topic. Must be implemented by the subclass.
   */
  virtual void advertiseImpl(
    std::shared_ptr<NodeType> nh,
    const std::string & base_topic,
    rmw_qos_profile_t custom_qos,
    rclcpp::PublisherOptions options) = 0;
};

}  // namespace image_transport

#endif  // IMAGE_TRANSPORT__PUBLISHER_PLUGIN_HPP_
