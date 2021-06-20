package ai.cvlab

import android.os.Handler
import android.os.Looper


// TODO: Create standalone calibration native state holder and its Kotlin wrapper
enum class CalibState {
    None, Viewing, Capturing, CapturePreview, Calculating
}

interface CalibStateUpdateReceiver {
    fun onCalibStateUpdate(event: CalibState, value: Int, error: String)
}

object Cvlab : Logger() {
    external fun create()
    external fun destroy()
    external fun startCamera()
    external fun stopCamera()

    private external fun startCalibration_jni()
    private external fun stopCalibration_jni()

    external fun startCalibCapture()
    external fun stopCalibCapture()

    external fun startCalibCalc()
    external fun stopCalibCalc()

    var calibState = CalibState.None
    private var calibStateUpdateReceiver: CalibStateUpdateReceiver? = null
    private var calibStateUpdateHandler: Handler? = null

    fun startCalibration(updateReceiver: CalibStateUpdateReceiver ) {
        calibStateUpdateReceiver = updateReceiver
        calibStateUpdateHandler = Handler(Looper.getMainLooper())
        startCalibration_jni()
        calibState = CalibState.None
    }

    fun stopCalibration() {
        stopCalibration_jni()
        calibStateUpdateReceiver = null
        calibStateUpdateHandler = null
        calibState = CalibState.None
    }

    @Suppress("unused")
    fun onCalibStateUpdate_jni(stateId: Int, value: Int, error: String) {
        calibStateUpdateHandler?.post {
            calibState = CalibState.values()[stateId]
            val errorPostfix = if (error.isEmpty())  "" else ". Error: $error"
            D("onCalibStateUpdate_jni: $calibState. Value: $value$errorPostfix")
            calibStateUpdateReceiver?.onCalibStateUpdate(calibState, value, error)
        }
    }
}

