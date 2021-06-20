package ai.cvlab

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import kotlinx.android.synthetic.main.fragment_calibration.*
import kotlinx.android.synthetic.main.fragment_calibration.view.*

// TODO: Rename parameter arguments, choose names that match
// the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
private const val ARG_PARAM1 = "param1"
private const val ARG_PARAM2 = "param2"

/**
 * A simple [Fragment] subclass.
 * Use the [CalibrationFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class CalibrationFragment : Fragment(), CalibStateUpdateReceiver {
    // TODO: Rename and change types of parameters
    private var param1: String? = null
    private var param2: String? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        arguments?.let {
            param1 = it.getString(ARG_PARAM1)
            param2 = it.getString(ARG_PARAM2)
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        D("onCreateView")

        val view = inflater.inflate(R.layout.fragment_calibration, container, false)

        view.calib_capture_button.setOnClickListener { onCalibCaptureClick(it) }
        view.calib_calc_button.setOnClickListener { onCalibCalcClick(it) }
        view.calib_calc_button.visibility = View.INVISIBLE

        Cvlab.startCamera()
        Cvlab.startCalibration(this)
        // Inflate the layout for this fragment
        return view
    }

    override fun onCalibStateUpdate(event: CalibState, value: Int, error: String) {
        val errorPostfix = if (error.isEmpty()) "" else ". Error: $error"
        D("onCalibStateUpdate: $event. Value: $value$errorPostfix")
        var status = ""
        when (event) {
            CalibState.None -> {
                // Not started yet
            }
            CalibState.Viewing -> {
                view?.calib_capture_button?.visibility = View.VISIBLE
                view?.calib_calc_button?.visibility = View.INVISIBLE
                view?.calib_capture_button?.text = getString(R.string.start_capturing)
            }
            CalibState.Capturing -> {
                // First time
                if (value == 0) {
                    view?.calib_capture_button?.visibility = View.VISIBLE
                    view?.calib_calc_button?.visibility = View.INVISIBLE
                    view?.calib_capture_button?.text = getString(R.string.stop_capturing)
                }
            }
            CalibState.CapturePreview -> {
//                if (value == 0) {
                    view?.calib_capture_button?.visibility = View.VISIBLE
                    view?.calib_calc_button?.visibility = View.VISIBLE
                    view?.calib_capture_button?.text = getString(R.string.start_capturing)
                    view?.calib_calc_button?.text = getText(R.string.calculate)
//                }
            }
            CalibState.Calculating -> {
                if (value == 0) {
                    view?.calib_capture_button?.visibility = View.INVISIBLE
                    view?.calib_calc_button?.visibility = View.VISIBLE
                    view?.calib_calc_button?.text = getText(R.string.stop_calculation)
                }
                status = "$value%"
            }
        }
        view?.calib_status_text?.visibility = View.INVISIBLE
        if (status.isNotEmpty()) {
            D("status: $status")
            view?.calib_status_text?.visibility = View.VISIBLE
            view?.calib_status_text?.text = status
        }

        if (error.isNotEmpty()) {
            Toast.makeText(context, error, Toast.LENGTH_LONG).show();
        }
    }

    private fun onCalibCaptureClick(view: View) {
        if (Cvlab.calibState == CalibState.Viewing || Cvlab.calibState == CalibState.CapturePreview) {
            Cvlab.startCalibCapture()
        } else if (Cvlab.calibState == CalibState.Capturing) {
            Cvlab.stopCalibCapture()
        }
    }

    private fun onCalibCalcClick(view: View) {
        if (Cvlab.calibState == CalibState.CapturePreview) {
            Cvlab.startCalibCalc()
        } else if (Cvlab.calibState == CalibState.Calculating) {
            Cvlab.stopCalibCalc()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        D("onDestroy")
        Cvlab.stopCamera()
        Cvlab.stopCalibration()
    }

    companion object : Logger() {
        /**
         * Use this factory method to create a new instance of
         * this fragment using the provided parameters.
         *
         * @param param1 Parameter 1.
         * @param param2 Parameter 2.
         * @return A new instance of fragment CalibrationFragment.
         */
        // TODO: Rename and change types and number of parameters
        @JvmStatic
        fun newInstance(param1: String, param2: String) =
            CalibrationFragment().apply {
                arguments = Bundle().apply {
                    putString(ARG_PARAM1, param1)
                    putString(ARG_PARAM2, param2)
                }
            }
    }


}