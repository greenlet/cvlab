#include "camera_android.h"

#include <camera/NdkCameraManager.h>

void CameraAndroid::start() {
    ACameraManager *camer_manager = ACameraManager_create();

    ACameraManager_delete(camer_manager);
}
