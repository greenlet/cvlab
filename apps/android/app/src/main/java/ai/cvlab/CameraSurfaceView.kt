package ai.cvlab

import android.content.Context
import android.opengl.GLSurfaceView
import android.util.AttributeSet

class CameraSurfaceView : GLSurfaceView {

    var camRenderer: CameraRenderer

    constructor(context: Context?, attrs: AttributeSet) : super(context, attrs) {
        setEGLContextClientVersion(2)

        camRenderer = CameraRenderer()
        setRenderer(camRenderer)
    }
}
