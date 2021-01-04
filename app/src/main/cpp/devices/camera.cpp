#include <devices/camera.hpp>

#include <utilities/log.hpp>

#include <stdexcept>
#include <sstream>
#include <string>

using namespace ::std;
using namespace ::utilities;

namespace devices
{
    camera::camera(ANativeWindow* a_window)
        : m_manager{nullptr, ACameraManager_delete},
          m_ids{nullptr, ACameraManager_deleteCameraIdList},
          m_device{nullptr, ACameraDevice_close},
          m_outputs{nullptr, ACaptureSessionOutputContainer_free},
          m_img_reader_output{nullptr, ACaptureSessionOutput_free},
          m_session{nullptr, ACameraCaptureSession_close},
          m_capture_req{nullptr, ACaptureRequest_free},
          m_target{nullptr, ACameraOutputTarget_free}
    {
        if(!a_window)
            throw runtime_error("Invalid window handle.");

        m_manager.reset(ACameraManager_create());

        if(!m_manager.get())
            throw runtime_error("Cannot create camera manager.");

        auto pt = m_ids.release();
        auto result = ACameraManager_getCameraIdList(m_manager.get(), &pt);
        m_ids.reset(pt);

        if(result != ACAMERA_OK)
        {
            stringstream sstr;
            sstr << "Failed to acquire camera list. (code: " << result << ").";
            throw runtime_error(sstr.str().c_str());
        }

        if(m_ids->numCameras < 1)
            throw runtime_error("No cameras found.");

        string selected_camera;

        for(uint32_t i = 0; i < m_ids->numCameras; ++i)
        {
            auto cam_id = m_ids->cameraIds[i];
            ACameraMetadata* metadata = nullptr;
            result = ACameraManager_getCameraCharacteristics(m_manager.get(), cam_id, &metadata);
            if(result != ACAMERA_OK || !metadata)
                continue;
            ACameraMetadata_const_entry entry;
            if(ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry) == ACAMERA_OK)
            {
                auto facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(entry.data.u8[0]);
                if(facing == ACAMERA_LENS_FACING_BACK)
                    selected_camera = string(cam_id);
            }
            ACameraMetadata_free(metadata);
        }

        if(selected_camera.size() < 1)
            throw runtime_error("Cannot find appropriate device.");

        {
            auto pt = m_device.release();
            result = ACameraManager_openCamera(m_manager.get(), selected_camera.c_str(),
                                               &m_dev_state_cbs, &pt);
            m_device.reset(pt);
        }

        if(result != ACAMERA_OK || !m_device.get())
        {
            stringstream sstr;
            sstr << "Couldn't open camera. (code: " << result << ").";
            throw runtime_error(sstr.str());
        }

        {
            auto pt = m_outputs.release();
            result = ACaptureSessionOutputContainer_create(&pt);
            m_outputs.reset(pt);
        }

        if(result != ACAMERA_OK)
            throw runtime_error("Capture session output container creation failed.");

        {
            auto pt = m_img_reader_output.release();
            result = ACaptureSessionOutput_create(a_window, &pt);
            m_img_reader_output.reset(pt);
        }

        if(result != ACAMERA_OK)
            throw runtime_error("Capture session image reader output creation failed.");

        result = ACaptureSessionOutputContainer_add(m_outputs.get(), m_img_reader_output.get());

        if(result != ACAMERA_OK)
            throw runtime_error("Couldn't add image reader output session to container.");

        {
            auto pt = m_session.release();
            result = ACameraDevice_createCaptureSession(m_device.get(), m_outputs.get(),
                                                        &m_cap_state_cbs, &pt);
            m_session.reset(pt);
        }

        if(result != ACAMERA_OK)
            throw runtime_error("Couldn't create capture session.");

        {
            auto pt = m_capture_req.release();
            result = ACameraDevice_createCaptureRequest(m_device.get(), TEMPLATE_PREVIEW, &pt);
            m_capture_req.reset(pt);
        }

        if(result != ACAMERA_OK)
            throw runtime_error("Couldn't create capture request.");

        {
            auto pt = m_target.release();
            result = ACameraOutputTarget_create(a_window, &pt);
            m_target.reset(pt);
        }

        if(result != ACAMERA_OK)
            throw runtime_error("Couldn't create camera output target.");

        result = ACaptureRequest_addTarget(m_capture_req.get(), m_target.get());

        if(result != ACAMERA_OK)
            throw runtime_error("Couldn't add capture request to camera output target.");

        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Camera logical device created.";
    }

    camera::~camera()
    {
        if constexpr(__ncv_logging_enabled)
            _log_android(log_level::info) << "Destroying camera device...";
    }

    void camera::start_capturing()
    {
        auto pt = m_capture_req.release();
        ACameraCaptureSession_setRepeatingRequest(m_session.get(), nullptr, 1, &pt, nullptr);
        m_capture_req.reset(pt);
    }

    void camera::stop_capturing()
    {
        ACameraCaptureSession_stopRepeating(m_session.get());
    }
}