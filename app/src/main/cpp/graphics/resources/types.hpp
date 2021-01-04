#ifndef NCV_RESOURCES_TYPES_HPP
#define NCV_RESOURCES_TYPES_HPP

namespace graphics{ namespace resources
{
    struct host;
    struct device_upload;
    struct device;
    struct external;

    template<typename T, typename S = void>
    class buffer;

    template<typename T, typename S = void>
    class image;
}}

#endif //NCV_RESOURCES_TYPES_HPP
