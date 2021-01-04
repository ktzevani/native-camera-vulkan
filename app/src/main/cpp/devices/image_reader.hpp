#ifndef NCV_IMAGE_READER_HPP
#define NCV_IMAGE_READER_HPP

#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <vector>

namespace devices
{
    class image_reader
    {
    public:
        
        using image_ptr = std::unique_ptr<AImage, decltype(&AImage_delete)>;
        using img_reader_ptr = std::unique_ptr<AImageReader, decltype(&AImageReader_delete)>;

        image_reader(uint32_t a_width, uint32_t a_height, uint32_t a_format, uint64_t a_usage,
            uint32_t a_max_images);

        ~image_reader();

        AHardwareBuffer* get_latest_buffer();
        ANativeWindow* get_window() const;

    private:

        uint32_t m_cur_index;
        ANativeWindow* m_window = nullptr;
        std::vector<AHardwareBuffer*> m_buffers;

        img_reader_ptr m_reader;
        std::vector<image_ptr> m_images;
    };
}

#endif // NCV_IMAGE_READER_HPP