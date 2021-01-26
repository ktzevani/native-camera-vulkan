/*
 * Copyright 2020 Konstantinos Tzevanidis
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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