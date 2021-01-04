#ifndef NCV_CAMERA_HPP
#define NCV_CAMERA_HPP

#include <camera/NdkCaptureRequest.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraManager.h>

#include <memory>

namespace devices
{
    class camera
    {
    public:

        using ACameraManager_ptr = std::unique_ptr<ACameraManager, decltype(&ACameraManager_delete)>;
        using ACameraIdList_ptr = std::unique_ptr<ACameraIdList, decltype(&ACameraManager_deleteCameraIdList)>;
        using ACameraDevice_ptr = std::unique_ptr<ACameraDevice, decltype(&ACameraDevice_close)>;
        using ACaptureSessionOutputContainer_ptr =
            std::unique_ptr<ACaptureSessionOutputContainer, decltype(&ACaptureSessionOutputContainer_free)>;
        using ACaptureSessionOutput_ptr =
            std::unique_ptr<ACaptureSessionOutput, decltype(&ACaptureSessionOutput_free)>;
        using ACameraCaptureSession_ptr =
            std::unique_ptr<ACameraCaptureSession, decltype(&ACameraCaptureSession_close)>;
        using ACaptureRequest_ptr = std::unique_ptr<ACaptureRequest, decltype(&ACaptureRequest_free)>;
        using ACameraOutputTarget_ptr =
            std::unique_ptr<ACameraOutputTarget, decltype(&ACameraOutputTarget_free)>;

        camera(ANativeWindow* a_window);
        ~camera();

        void start_capturing();
        void stop_capturing();

        static void on_device_disconnected(void* a_obj, ACameraDevice* a_device)
        {}

        static void on_device_error(void* a_obj, ACameraDevice* a_device, int a_err_code)
        {}

        static void on_session_closed(void* a_obj, ACameraCaptureSession* a_session)
        {}

        static void on_session_ready(void* a_obj, ACameraCaptureSession* a_session)
        {}

        static void on_session_active(void* a_obj, ACameraCaptureSession* a_session)
        {}

    private:

        ACameraDevice_stateCallbacks m_dev_state_cbs
        {
            this,
            on_device_disconnected,
            on_device_error
        };

        ACameraCaptureSession_stateCallbacks m_cap_state_cbs
        {
            this,
            on_session_closed,
            on_session_ready,
            on_session_active
        };

        ACameraManager_ptr m_manager;
        ACameraIdList_ptr m_ids;
        ACameraDevice_ptr m_device;
        ACaptureSessionOutputContainer_ptr m_outputs;
        ACaptureSessionOutput_ptr m_img_reader_output;
        ACameraCaptureSession_ptr m_session;
        ACaptureRequest_ptr m_capture_req;
        ACameraOutputTarget_ptr m_target;
    };

}

#endif //NCV_CAMERA_HPP
