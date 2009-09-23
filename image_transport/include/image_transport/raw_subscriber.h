#ifndef IMAGE_TRANSPORT_RAW_SUBSCRIBER_H
#define IMAGE_TRANSPORT_RAW_SUBSCRIBER_H

#include "image_transport/subscriber_plugin.h"

namespace image_transport {

/**
 * \brief The default SubscriberPlugin.
 *
 * RawSubscriber is a simple wrapper for ros::Subscriber which listens for Image messages
 * and passes them through to the callback.
 */
class RawSubscriber : public SubscriberPlugin
{
public:
  RawSubscriber();
  
  virtual ~RawSubscriber();

  virtual std::string getTransportName() const;
  
  virtual std::string getTopic() const;

  virtual void shutdown();

protected:
  virtual void subscribeImpl(ros::NodeHandle& nh, const std::string& base_topic, uint32_t queue_size,
                             const boost::function<void(const sensor_msgs::ImageConstPtr&)>& callback,
                             const ros::VoidPtr& tracked_object, const ros::TransportHints& transport_hints);
  
private:
  ros::Subscriber sub_;
};

} //namespace image_transport

#endif