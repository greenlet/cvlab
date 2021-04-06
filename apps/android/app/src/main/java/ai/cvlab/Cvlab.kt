package ai.cvlab

import android.os.Handler
import android.os.Looper


// TODO: Create standalone calibration native state holder and its Kotlin wrapper
enum class CalibState {
    Viewing, Capturing, CapturePreview, Calculating
}

interface CalibStateUpdateReceiver {
    fun onCalibStateUpdate(event: CalibState, value: Int)
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

    var calibState = CalibState.Viewing
    private var calibStateUpdateReceiver: CalibStateUpdateReceiver? = null
    private var calibStateUpdateHandler: Handler? = null

    fun startCalibration(updateReceiver: CalibStateUpdateReceiver ) {
        calibStateUpdateReceiver = updateReceiver
        calibStateUpdateHandler = Handler(Looper.getMainLooper())
        startCalibration_jni()
        calibState = CalibState.Viewing
    }

    fun stopCalibration() {
        stopCalibration_jni()
        calibStateUpdateReceiver = null
        calibStateUpdateHandler = null
        calibState = CalibState.Viewing
    }

    @Suppress("unused")
    fun onCalibStateUpdate_jni(stateId: Int, value: Int) {
        calibState = CalibState.values()[stateId]
        D("onCalibStateUpdate_jni: $calibState. Value: $value")
        calibStateUpdateHandler?.post {
            calibStateUpdateReceiver?.onCalibStateUpdate(calibState, value)
        }
    }
}

