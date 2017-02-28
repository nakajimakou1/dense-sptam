#include "PointCloudQueue.hpp"

PointCloudEntry::PointCloudEntry(uint32_t seq)
  : seq_(seq)
  , current_pos_(nullptr)
  , update_pos_(nullptr)
  , disp_raw_img_(nullptr)
  , points_mat_(nullptr)
  , cloud_(nullptr)
  , state_(LOCAL_MAP)
{}

PointCloudEntry::~PointCloudEntry()
{}

int PointCloudEntry::save_cloud(const char *output_dir)
{
    char filename[128];

    if (this->get_cloud() == nullptr || this->get_cloud()->size() == 0)
        return -1;

    sprintf(filename, "%s/cloud_%06u.pcd", output_dir, this->get_seq());
    pcl::io::savePCDFileBinary(filename, *this->get_cloud());
    this->set_cloud(nullptr);

    sprintf(filename, "%s/cloud_%06u.txt", output_dir, this->get_seq());
    CameraPose::Ptr current_pose = this->get_current_pos();
    if (current_pose->save(filename) < 0)
        return -1;

    return 0;
}

int PointCloudEntry::load_cloud(const char *output_dir)
{
    PointCloudPtr cloud(new PointCloud);
    char filename[128];

    sprintf(filename, "%s/cloud_%06u.pcd", output_dir, this->get_seq());
    if (pcl::io::loadPCDFile(filename, *cloud) < 0)
        return -1;

    this->set_cloud(cloud);

    return 0;
}

PointCloudQueue::PointCloudQueue(unsigned max_local_area_size)
  : max_local_area_size_(max_local_area_size)
{}

PointCloudQueue::~PointCloudQueue()
{}

PointCloudEntry::Ptr PointCloudQueue::getEntry(uint32_t seq, bool force)
{
    std::lock_guard<std::mutex> lock(vector_lock_);
    PointCloudEntry::Ptr& ret = entries_[seq];

    if (force && !ret)
        ret = boost::make_shared<PointCloudEntry>(seq);

    return ret;
}

size_t PointCloudQueue::size()
{
    std::lock_guard<std::mutex> lock(vector_lock_);
    return entries_.size();
}

uint32_t PointCloudQueue::get_local_area_seq()
{
    if (local_area_queue_.size() > 0)
        return local_area_queue_.back()->get_seq();
    else
        return 0;
}

void PointCloudQueue::save_all(const char *output_dir)
{
    for (auto& it : entries_) {
        if (it.second != nullptr && it.second->save_cloud(output_dir) == 0)
            std::cout << "Saved cloud seq " << it.second->get_seq() << std::endl;
    }
}

void PointCloudQueue::get_local_area_cloud(double pub_area_filter_min,
                                           PointCloudPtr ret_good, PointCloudPtr ret_bad)
{
    ret_good->header.seq = ret_bad->header.seq = 0;
    for (auto& it : local_area_queue_) {
        if (it->get_cloud() != nullptr) {
            for (auto& p : *it->get_cloud()) {
                if (p.a >= pub_area_filter_min)
                    ret_good->push_back(p);
                else
                    ret_bad->push_back(p);
            }
            ret_good->header.seq = ret_bad->header.seq = it->get_seq();
        }
    }
}

void PointCloudQueue::push_local_area(PointCloudEntry::Ptr entry)
{
    if (local_area_queue_.size() > max_local_area_size_) {
        PointCloudEntry::Ptr entry = local_area_queue_.front();
        entry->set_state(PointCloudEntry::GLOBAL_MAP_RAM);

        local_area_queue_.erase(local_area_queue_.begin());
    }

    local_area_queue_.push_back(entry);
}
