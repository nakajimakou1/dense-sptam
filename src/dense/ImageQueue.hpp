#ifndef __IMAGE_QUEUE_H
#define __IMAGE_QUEUE_H

#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <mutex>
#include <condition_variable>
#include <queue>

typedef sensor_msgs::Image Image;
typedef boost::shared_ptr<Image> ImagePtr;

class ImageQueue
{
public:

    ImageQueue();
    ~ImageQueue();

    void push(ImagePtr image);
    ImagePtr pop();
    size_t size();

private:

    std::mutex image_queue_lock_;
    std::condition_variable empty_queue_cv;
    std::queue<ImagePtr> images_;
};

#endif /* __IMAGE_QUEUE_H */
