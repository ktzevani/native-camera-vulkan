#include <vk_util/vk_helpers.hpp>

using namespace ::std;
using namespace ::vk;
using namespace ::utilities;

namespace vk_util
{
    VKAPI_ATTR Bool32 VKAPI_CALL message_callback(DebugUtilsMessageSeverityFlagBitsEXT a_severity,
          DebugUtilsMessageTypeFlagsEXT a_msg_type,
          const DebugUtilsMessengerCallbackDataEXT *a_dbg_data,
          void* a_user_data)
    {
        string general_type = "General";
        string performance_type = "Performance";
        string validation_type = "Validation";
        string msg_type;

        if(a_msg_type & DebugUtilsMessageTypeFlagBitsEXT::eGeneral)
            msg_type = general_type;
        else if(a_msg_type & DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
            msg_type = performance_type;
        else if(a_msg_type & DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            msg_type = validation_type;

        if(a_severity & DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            _log_android(log_level::info) << msg_type << " - " << a_dbg_data->pMessage;
        else if(a_severity & DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            _log_android(log_level::warning) << msg_type << " - " << a_dbg_data->pMessage;
        else if(a_severity & DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
            _log_android(log_level::debug) << msg_type << " - " << a_dbg_data->pMessage;
        else
        {
            // Special treatment for a specific validation error in order to avoid premature app exit
            // when validation layers are enabled. For an explanation read the following.
            //
            // It seems that for the camera-specific format (vulkan image resource external format = 506)
            // of the development device (Nokia 6.1, Adreno 508, Driver 512.415.0, Vulkan API Version 1.1.87),
            // the feature flag VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT is missing as reported by the hardware
            // buffer format properties. Although this external format seems to be lacking the mentioned
            // feature, the application works if the validation check is omitted. This is weird because
            // in order for an external image buffer of unknown to vulkan format, to be mapped in vulkan
            // space and consumed inside a vulkan context (via the use of a ycbcr sampler), its format
            // MUST support sampling feature.
            //
            // For more details see below:
            // - https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#samplers-YCbCr-conversion
            // - https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-VkImageViewCreateInfo-usage-02274
            //
            // Maybe this is an error in camera format definitions of the specific device. Don't have
            // the resources to test this in other devices (To be investigated).

            string msg_str(a_dbg_data->pMessage);

            array<string, 3> VUID_02274 = {
                    "VUID-VkImageViewCreateInfo-usage-02274",
                    "VK_FORMAT_UNDEFINED",
                    "VK_IMAGE_TILING_OPTIMAL"
            };

            auto ret_flag = VK_TRUE;

            if(msg_str.find(VUID_02274[0]) != string::npos)
            {
                if(msg_str.find(VUID_02274[1]) != string::npos && msg_str.find(VUID_02274[2]) != string::npos)
                    ret_flag = VK_FALSE;
            }

            if(ret_flag)
                _log_android(log_level::error) << msg_type << " - " << a_dbg_data->pMessage;
            else
                _log_android(log_level::verbose) << msg_type << " - " << a_dbg_data->pMessage;

            return ret_flag;
        }

        return VK_FALSE;
    }

    string readable_size(size_t a_size)
    {
        static const vector<string> suffixes{ { "B", "KB", "MB", "GB", "TB", "PB" } };
        size_t suffixIndex = 0;
        while (suffixIndex < suffixes.size() - 1 && a_size > 1024)
        {
            a_size >>= 10;
            ++suffixIndex;
        }
        stringstream buffer;
        buffer << a_size << " " << suffixes[suffixIndex];
        return buffer.str();
    }

    perf_timer::perf_timer(const string &a_func)
        : m_func_name{a_func}, m_clock{std::chrono::high_resolution_clock::now()}
    {}

    perf_timer::~perf_timer()
    {
        using namespace std::chrono;
        auto ns = duration_cast<nanoseconds>(high_resolution_clock::now() - m_clock);
        _log_android(log_level::verbose)
            << "Function name: " << m_func_name << " - Time elapsed: " << ns.count() << " ns ("
            << 1.0e9 / static_cast<double>(ns.count()) << " Hz)";
    }

    u_long perf_timer::elapsed_ns() const
    {
        using namespace std::chrono;
        return (duration_cast<nanoseconds>(high_resolution_clock::now() - m_clock)).count();
    }

    logger::logger(uint32_t a_box_width, uint32_t a_col_width, log_level a_log_level)
        : m_bw{a_box_width}, m_cw{a_col_width}, m_level{a_log_level}
    {}

    void logger::box_top() const
    {
        box_line("┌", "┐");
    }

    void logger::box_bottom() const
    {
        box_line("└", "┘");
    }

    void logger::box_separator() const
    {
        box_line();
    }

    void logger::write_multiline(const char *a_str, const vector<string> &a_vec, char a_fill) const
    {
        uint32_t char_count = 0, str_len = strlen(a_str);

        for(uint32_t i = 0; i < str_len; ++i)
            char_count += (a_str[i] & 0xC0) != 0x80;

        int w = m_bw - 1 + strlen(a_str) - char_count;
        auto l = m_level;

        vector<string> lines;
        uint32_t cln {char_count};
        stringstream sstr;

        for(auto& str : a_vec)
        {
            cln += str.length() + 3;
            if(cln > m_bw-2)
            {
                lines.push_back(sstr.str());
                sstr.str("");
                cln = char_count + str.length() + 3;
            }
            sstr << str << " | ";
        }

        auto str = sstr.str();

        if(a_vec.size() < 1)
            str = "{}";
        else
            str.resize(str.size()-3);

        lines.push_back(str);
        str = string(a_str);

        for(auto& wd : lines)
        {
            _log_android(l)
                    << "|" << setfill(a_fill) << setw(w) << left << (str + wd).c_str() << "|";
            str = string(char_count, ' ');
        }
    }

    void logger::write_line(const char *a_str, int a_align, char a_fill) const
    {
        uint32_t char_count = 0, str_len = strlen(a_str);

        for(uint32_t i = 0; i < str_len; ++i)
            char_count += (a_str[i] & 0xC0) != 0x80;

        int w = m_bw - 1 + strlen(a_str) - char_count;
        auto l = m_level;

        if (a_align == 0)
            _log_android(l) << "|" << setfill(a_fill) << setw(w) << left << a_str << "|";
        else if (a_align == 1)
            _log_android(l) << "|" << setfill(a_fill) << setw(w) << centered(a_str) << "|";
        else
            _log_android(l) << "|" << setfill(a_fill) << setw(w) << right << a_str << "|";
    }

    void logger::write_line_2col(const char *a_lstr, const char *a_rstr, int a_align, char a_fill) const
    {
        uint32_t lchar_count = 0, rchar_count = 0, lstr_len = strlen(a_lstr),
                rstr_len = strlen(a_rstr);

        for(uint32_t i = 0; i < lstr_len; ++i)
            lchar_count += (a_lstr[i] & 0xC0) != 0x80;

        for(uint32_t i = 0; i < rstr_len; ++i)
            rchar_count += (a_rstr[i] & 0xC0) != 0x80;

        int sw = m_cw + strlen(a_rstr) - rchar_count,
                w = m_bw - m_cw - 1 + strlen(a_lstr) - lchar_count;
        auto l = m_level;
        char c = a_fill;

        if (a_align == 0)
            _log_android(l) << "|" << setfill(c) << setw(w) << left << a_lstr << setfill(c)
                            << setw(sw) << left << a_rstr << "|";
        else if (a_align == 1)
            _log_android(l) << "|" << setfill(c) << setw(w) << centered(a_lstr) << setfill(c)
                            << setw(sw) << centered(a_rstr) << "|";
        else
            _log_android(l) << "|" << setfill(c) << setw(w) << right << a_lstr << setfill(c)
                            << setw(sw) << right << a_rstr << "|";
    }

    void logger::box_line(const char *a_start, const char *a_end, char a_fill) const
    {
        int w = m_bw + strlen(a_end) - 1;
        auto l = m_level;
        _log_android(l) << a_start << setfill(a_fill) << setw(w) << a_end;
    }
}