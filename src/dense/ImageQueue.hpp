#ifndef __IMAGE_QUEUE_H
#define __IMAGE_QUEUE_H

#include <sensor_msgs/Image.h>
#include <condition_variable>
#include <mutex>
#include <queue>

typedef sensor_msgs::Image Image;
typedef boost::shared_ptr<Image> ImagePtr;

typedef std::pair<ImagePtr, ImagePtr> ImagePair;
typedef boost::shared_ptr<ImagePair> ImagePairPtr;

class ImageQueue
{
public:

    ImageQueue();
    ~ImageQueue();

    void push(ImagePairPtr image);
    ImagePairPtr pop(bool remove = true);
    size_t size();

private:

    std::mutex image_queue_lock_;
    std::condition_variable empty_queue_cv;
    std::queue<ImagePairPtr> images_;
};

#endif /* __IMAGE_QUEUE_H */