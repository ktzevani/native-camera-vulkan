#ifndef NCV_TEXTURE_HPP
#define NCV_TEXTURE_HPP

#include <string>
#include <vector>

class AAssetManager;

namespace graphics{ namespace data{

    class texture
    {
    public:
        typedef unsigned char stbi_uc;
        typedef struct
        {
            uint32_t width;
            uint32_t height;
            uint32_t depth;
        } extent3d;

        texture(AAssetManager* a_ass_mgr, const std::string& a_filename);
        std::vector<stbi_uc>& get_data() { return m_data; }
        extent3d get_extent() { return m_extent; }
    private:
        std::vector<stbi_uc> m_data;
        extent3d m_extent;
    };

}}

#endif //NCV_TEXTURE_HPP
