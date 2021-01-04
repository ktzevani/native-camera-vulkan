#ifndef NCV_ANDROID_PERMISSIONS_HPP
#define NCV_ANDROID_PERMISSIONS_HPP

class android_app;

namespace core
{
    class android_permissions
    {
    public:
        static bool is_camera_permitted(android_app *a_app);
        static void request_camera_permission(android_app *a_app);
    };
}

#endif //NCV_ANDROID_PERMISSIONS_HPP
