package ai.motion3d

import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES
import android.opengl.GLES20
import android.opengl.GLSurfaceView
import android.view.Surface
import javax.microedition.khronos.opengles.GL10
import javax.microedition.khronos.egl.EGLConfig;

class CameraRenderer: GLSurfaceView.Renderer {
    lateinit var surfaceTexture: SurfaceTexture
    val texMatrix = FloatArray(16)
    @Volatile var frameAvailable: Boolean = false
    val lock = Object()

    override fun onSurfaceCreated(p0: GL10?, p1: EGLConfig?) {
        // Prepare texture and surface
        val textures = IntArray(1)
        GLES20.glGenTextures(1, textures, 0)
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures[0])

        surfaceTexture = SurfaceTexture(textures[0])
        surfaceTexture.setOnFrameAvailableListener {
            synchronized(lock) {
                frameAvailable = true
            }
        }

        // Choose you preferred preview size here before creating surface
        // val optimalSize = getOptimalSize()
        // surfaceTexture.setDefaultBufferSize(optimalSize.width, optimalSize.height)
        surfaceTexture.setDefaultBufferSize(1920, 1080)

        val surface = Surface(surfaceTexture)

        // Pass to native code
        onSurfaceCreated(textures[0], surface)
    }

    override fun onSurfaceChanged(p0: GL10?, width: Int, height: Int) {
        onSurfaceChanged(width, height)
    }

    override fun onDrawFrame(p0: GL10?) {
        synchronized(lock) {
            if (frameAvailable) {
                surfaceTexture.updateTexImage()
                surfaceTexture.getTransformMatrix(texMatrix)
                frameAvailable = false
            }
        }

        onDrawFrame(texMatrix)
    }

    private external fun onSurfaceCreated(textureId: Int, surface: Surface)
    private external fun onSurfaceChanged(width: Int, height: Int)
    private external fun onDrawFrame(texMat: FloatArray)
}

