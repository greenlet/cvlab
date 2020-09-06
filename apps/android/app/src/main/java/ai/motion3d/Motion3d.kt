package ai.motion3d

object Motion3d {
    private var nativeApp: Long = 0

    fun createApp() {
        nativeApp = create()
    }

    fun destroyApp() {
        destroy(nativeApp)
        nativeApp = 0
    }

    fun startCamera() {
        startCamera(nativeApp)
    }

    fun stopCamera() {
        startCamera(nativeApp)
    }

    private external fun create(): Long
    private external fun destroy(nativeApp: Long)
    private external fun startCamera(nativeApp: Long)
    private external fun stopCamera(nativeApp: Long)

}