#include "camera_android.h"

#include <camera/NdkCameraManager.h>

#include <thread>

CameraAndroid::CameraAndroid() : Logger("CameraAndroid") {

}

class CameraCallbacksTest {
public:
    void onDisconnected(ACameraDevice* device)
    {
        // ...
    }

    void onError(ACameraDevice* device, int error)
    {
        // ...
    }
};

void CameraAndroid::start() {
    ACameraManager *camera_manager = ACameraManager_create();
    ACameraIdList *camera_ids = nullptr;
    ACameraManager_getCameraIdList(camera_manager, &camera_ids);

    D("Cameras:");
    const char *back_camera_id = nullptr;
    for (int i = 0; i < camera_ids->numCameras; i++) {
        const char *camera_id = camera_ids->cameraIds[i];
        ACameraMetadata *camera_metadata;
        ACameraManager_getCameraCharacteristics(camera_manager, camera_id, &camera_metadata);
        D("Camera %s", camera_id);
        int32_t tags_n;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(camera_metadata, &tags_n, &tags);

        for (int tag_ind = 0; tag_ind < tags_n; tag_ind++) {
            if (tags[tag_ind] == ACAMERA_LENS_FACING) {
                ACameraMetadata_const_entry lens_info = {0};
                camera_status_t status = ACameraMetadata_getConstEntry(camera_metadata, tags[tag_ind], &lens_info);
                if (status != ACAMERA_OK) {
                    continue;
                }
                const uint8_t facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                        lens_info.data.u8[0]);
                if (facing == ACAMERA_LENS_FACING_BACK) {
                    D("Camera facing back!");
                    back_camera_id = camera_id;
                }
            }
        }

        ACameraMetadata_free(camera_metadata);
    }

//    CameraCallbacksTest cam_cb;
//    ACameraDevice_stateCallbacks camera_device_callbacks = {
//            .context = &cam_cb,
//            .onDisconnected = &CameraCallbacksTest::onDisconnected,
//    };
//
//
//    if (back_camera_id) {
//        ACameraManager_openCamera(camera_manager, back_camera_id)
//    }

    ACameraManager_deleteCameraIdList(camera_ids);
    ACameraManager_delete(camera_manager);
}
