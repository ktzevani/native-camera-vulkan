#include <devices/image_reader.hpp>

#include <utilities/log.hpp>

using namespace ::std;
using namespace ::utilities;

namespace devices
{
    image_reader::image_reader(uint32_t a_width, uint32_t a_height, uint32_t a_format, uint64_t a_usage,
        uint32_t a_max_images)
        : m_cur_index{a_max_images-1}, m_reader{nullptr, AImageReader_delete}
    {
        if(a_max_images < 2)
            throw runtime_error("Max images must be at least 2.");

        for(uint32_t i = 0; i < a_max_images; ++i)
            m_images.push_back({nullptr, AImage_delete});

        m_buffers = vector<AHardwareBuffer*>(a_max_images, nullptr);

        auto pt = m_reader.release();
        AImageReader_newWithUsage(a_width, a_height, a_format, a_usage, m_images.size()+2, &pt);
        m_reader.reset(pt);

        if(!m_reader)
            throw runtime_error("Failed to create image reader.");

        auto result = AImageReader_getWindow(m_reader.get(), &m_window);

        if (result != AMEDIA_OK || m_window == nullptr)
            throw runtime_error("Failed to obtain window handle.");

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Image reader created.";
    }

    image_reader::~image_reader()
    {
        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Destroying image reader...";
    }

    AHardwareBuffer * image_reader::get_latest_buffer()
    {
        AImage *image = nullptr;
        auto result = AImageReader_acquireLatestImage(m_reader.get(), &image);
        if(result != AMEDIA_OK || !image)
        {
            if constexpr(__ncv_logging_enabled)
                _log_android(log_level::verbose) << "Failed to acquire image from camera.";
        }
        else
        {
            AHardwareBuffer* buffer = nullptr;
            auto result = AImage_getHardwareBuffer(image, &buffer);
            if(result != AMEDIA_OK || !buffer)
            {
                if constexpr(__ncv_logging_enabled)
                    _log_android(log_level::verbose) << "Failed to acquire hardware buffer.";
            }
            else
            {
                m_cur_index++;
                if(m_cur_index == m_images.size())
                    m_cur_index = 0;
                m_images[m_cur_index].reset(image);
                m_buffers[m_cur_index] = buffer;
            }
        }

        return m_buffers[m_cur_index];
    }

    ANativeWindow * image_reader::get_window() const
    {
        return m_window;
    }
}