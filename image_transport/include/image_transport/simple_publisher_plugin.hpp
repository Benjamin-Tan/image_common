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

#ifndef IMAGE_TRANSPORT__SIMPLE_PUBLISHER_PLUGIN_HPP_
#define IMAGE_TRANSPORT__SIMPLE_PUBLISHER_PLUGIN_HPP_

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "rclcpp/node.hpp"
#include "rclcpp/logger.hpp"
#include "rclcpp/logging.hpp"

#include "image_transport/publisher_plugin.hpp"
#include "image_transport/visibility_control.hpp"

namespace image_transport
{

/**
 * \brief Base class to simplify implementing most plugins to Publisher.
 *
 * This base class vastly simplifies implementing a PublisherPlugin in the common
 * case that all communication with the matching SubscriberPlugin happens over a
 * single ROS topic using a transport-specific message type. SimplePublisherPlugin
 * is templated on the transport-specific message type.
 *
 * A subclass need implement only two methods:
 * - getTransportName() from PublisherPlugin
 * - publish() with an extra PublishFn argument
 *
 * For access to the parameter server and name remappings, use nh().
 *
 * getTopicToAdvertise() controls the name of the internal communication topic.
 * It defaults to \<base topic\>/\<transport name\>.
 */
template<class M, class NodeType = rclcpp::Node>
class SimplePublisherPlugin : public PublisherPlugin<NodeType>
{
public:
  virtual ~SimplePublisherPlugin() {}

  size_t getNumSubscribers() const override
  {
    if (simple_impl_) {
      return simple_impl_->pub_->get_subscription_count();
    }
    return 0;
  }

  std::string getTopic() const override
  {
    if (simple_impl_) {return simple_impl_->pub_->get_topic_name();}
    return std::string();
  }

  void publish(const sensor_msgs::msg::Image & message) const override
  {
    if (!simple_impl_ || !simple_impl_->pub_) {
      auto logger = simple_impl_ ? simple_impl_->logger_ : rclcpp::get_logger("image_transport");
      RCLCPP_ERROR(
        logger,
        "Call to publish() on an invalid image_transport::SimplePublisherPlugin");
      return;
    }

    publish(message, simple_impl_->pub_);
  }

  void publishUniquePtr(sensor_msgs::msg::Image::UniquePtr message) const override
  {
    if (!simple_impl_ || !simple_impl_->pub_) {
      auto logger = simple_impl_ ? simple_impl_->logger_ : rclcpp::get_logger("image_transport");
      RCLCPP_ERROR(
        logger,
        "Call to publish() on an invalid image_transport::SimplePublisherPlugin");
      return;
    }

    publish(std::move(message), simple_impl_->pub_);
  }

  void shutdown() override
  {
    simple_impl_.reset();
  }

protected:
  void advertiseImpl(
    std::shared_ptr<NodeType> nh,
    const std::string & base_topic,
    rmw_qos_profile_t custom_qos,
    rclcpp::PublisherOptions options) override
  {
    std::string transport_topic = getTopicToAdvertise(base_topic);
    auto qos = rclcpp::QoS(rclcpp::QoSInitialization::from_rmw(custom_qos), custom_qos);
    simple_impl_ = std::make_unique<SimplePublisherPluginImpl>(nh);
    simple_impl_->pub_ = simple_impl_->node_->template create_publisher<M>(
      transport_topic, qos, options);

    RCLCPP_DEBUG(simple_impl_->logger_, "getTopicToAdvertise: %s", transport_topic.c_str());
  }

  typedef typename rclcpp::Publisher<M>::SharedPtr PublisherT;

  //! Generic function for publishing the internal message type.
  typedef std::function<void (const M &)> PublishFn;

  /**
   * \brief Publish an image using the specified publish function.
   *
   * \deprecated Use publish(const sensor_msgs::msg::Image&, const PublisherT&) instead.
   *
   * The PublishFn publishes the transport-specific message type. This indirection allows
   * SimpleSubscriberPlugin to use this function for both normal broadcast publishing and
   * single subscriber publishing (in subscription callbacks).
   */
  virtual void publish(
    const sensor_msgs::msg::Image & /*message*/,
    const PublishFn & /*publish_fn*/) const
  {
    throw std::logic_error(
            "publish(const sensor_msgs::msg::Image&, const PublishFn&) is not implemented.");
  }

  /**
   * \brief Publish an image using the specified publisher.
   */
  virtual void publish(
    const sensor_msgs::msg::Image & message,
    const PublisherT & publisher) const
  {
    // Fallback to old, deprecated method
    publish(message, bindInternalPublisher(publisher.get()));
  }

  /**
   * \brief Publish an image using the specified publisher.
   *
   * This version of the function can be used to optimize cases where the Plugin can
   * avoid doing copies of the data when having the ownership to the image message.
   * Plugins that can take advantage of message ownership should overwrite this method
   * along with supportsUniquePtrPub().
   */
  virtual void publish(
    sensor_msgs::msg::Image::UniquePtr /*message*/,
    const PublisherT & /*publisher*/) const
  {
    throw std::logic_error(
            "publish(sensor_msgs::msg::Image::UniquePtr, const PublisherT&) is not implemented.");
  }

  /**
   * \brief Return the communication topic name for a given base topic.
   *
   * Defaults to \<base topic\>/\<transport name\>.
   */
  virtual std::string getTopicToAdvertise(const std::string & base_topic) const
  {
    return base_topic + "/" + PublisherPlugin<NodeType>::getTransportName();
  }

private:
  struct SimplePublisherPluginImpl
  {
    explicit SimplePublisherPluginImpl(NodeType * node)
    : node_(node),
      logger_(node->get_logger())
    {
    }

    explicit SimplePublisherPluginImpl(std::shared_ptr<NodeType> node)
    : node_(node),
      logger_(node->get_logger())
    {
    }

    std::shared_ptr<NodeType> node_;
    rclcpp::Logger logger_;
    PublisherT pub_;
  };

  std::unique_ptr<SimplePublisherPluginImpl> simple_impl_;

  typedef std::function<void (const sensor_msgs::msg::Image &)> ImagePublishFn;

  /**
   * Returns a function object for publishing the transport-specific message type
   * through some ROS publisher type.
   *
   * @param pub An object with method void publish(const M&)
   */
  template<class PubT>
  PublishFn bindInternalPublisher(PubT * pub) const
  {
    // Bind PubT::publish(const Message&) as PublishFn
    typedef void (PubT::* InternalPublishMemFn)(const M &);
    InternalPublishMemFn internal_pub_mem_fn = &PubT::publish;
    return std::bind(internal_pub_mem_fn, pub, std::placeholders::_1);
  }
};

}  // namespace image_transport

#endif  // IMAGE_TRANSPORT__SIMPLE_PUBLISHER_PLUGIN_HPP_
