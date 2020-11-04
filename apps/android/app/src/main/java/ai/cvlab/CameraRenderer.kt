package ai.cvlab

import android.opengl.GLSurfaceView
import javax.microedition.khronos.opengles.GL10
import javax.microedition.khronos.egl.EGLConfig;

class CameraRenderer: GLSurfaceView.Renderer {
    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        onSurfaceCreated()
    }

    override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
        onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(p0: GL10?) {
        onDrawFrame()
    }

    private external fun onSurfaceCreated()
    private external fun onSurfaceChanged(width: Int, height: Int)
    private external fun onDrawFrame()
}

