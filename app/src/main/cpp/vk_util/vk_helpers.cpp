#include <vk_util/vk_helpers.hpp>

using namespace ::std;
using namespace ::vk;
using namespace ::utilities;

namespace vk_util
{
    VKAPI_ATTR Bool32 VKAPI_CALL message_callback(DebugReportFlagsEXT a_flags,
        DebugReportObjectTypeEXT a_obj_t, uint64_t a_src_obj, size_t a_location, int32_t a_msg_code,
        const char* a_layer_prefix, const char* a_msg, void* a_user_data)
    {
        if(a_flags & DebugReportFlagBitsEXT::eInformation)
            _log_android(log_level::info) << a_layer_prefix << " - " << a_msg;
        else if(a_flags & DebugReportFlagBitsEXT::eWarning)
        {
            bool log_verbose = false;
            string msg_str(a_msg);
            array<string, 4> filters = {
                    "[ VUID-VkSamplerCreateInfo-pNext-pNext ]",
                    "[ VUID-VkImageViewCreateInfo-pNext-pNext ]",
                    "VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID",
                    "VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO"
            };

            if(msg_str.find(filters[0]) != string::npos ||
               msg_str.find(filters[1]) != string::npos)
            {
                if(msg_str.find(filters[2]) != string::npos ||
                   ((msg_str.find(filters[1]) != string::npos) &&
                    (msg_str.find(filters[3]) != string::npos)))
                    log_verbose = true;
            }

            if(log_verbose)
                _log_android(log_level::verbose) << a_layer_prefix << " - " << a_msg;
            else
                _log_android(log_level::warning) << a_layer_prefix << " - " << a_msg;
        }
        else if(a_flags & DebugReportFlagBitsEXT::eDebug)
            _log_android(log_level::debug) << a_layer_prefix << " - " << a_msg;
        else if(a_flags & DebugReportFlagBitsEXT::ePerformanceWarning)
            _log_android(log_level::verbose) << a_layer_prefix << " - " << a_msg;
        else
        {
            // Special treatment for specific validation errors in order to avoid premature app
            // termination. See similar issue reported at the following link
            // https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/1935
            // For some reason external memory resource allocation and binding doesn't pass validation
            // although it seems to be working fine. My research didn't produce a solution other than
            // the following workaround.

            string msg_str(a_msg);
            array<string, 4> VUID_01881 = {
                    "VUID-VkImportAndroidHardwareBufferInfoANDROID-buffer-01881",
                    "format ( 34 )",
                    "format ( 35 )",
                    "usage ( 0x20100 )"
            };
            array<string, 3> VUID_02273 = {
                    "VUID-VkImageViewCreateInfo-None-02273",
                    "VK_FORMAT_UNDEFINED",
                    "VK_IMAGE_TILING_OPTIMAL"
            };

            auto ret_flag = VK_TRUE;

            if(msg_str.find(VUID_01881[0]) != string::npos)
            {
                if(msg_str.find(VUID_01881[3]) != string::npos &&
                   (msg_str.find(VUID_01881[1]) != string::npos ||
                    msg_str.find(VUID_01881[2]) != string::npos))
                    ret_flag = VK_FALSE;
            }
            else if(msg_str.find(VUID_02273[0]) != string::npos)
            {
                if(msg_str.find(VUID_02273[1]) != string::npos &&
                   msg_str.find(VUID_02273[2]) != string::npos)
                    ret_flag = VK_FALSE;
            }

            if(ret_flag)
                _log_android(log_level::error) << a_layer_prefix << " - " << a_msg;
            else
                _log_android(log_level::verbose) << a_layer_prefix << " - " << a_msg;

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