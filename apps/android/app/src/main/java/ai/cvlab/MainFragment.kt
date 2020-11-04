package ai.cvlab

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.navigation.findNavController
import kotlinx.android.synthetic.main.fragment_main.*
import kotlinx.android.synthetic.main.fragment_main.view.*

// TODO: Rename parameter arguments, choose names that match
// the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
private const val ARG_PARAM1 = "param1"
private const val ARG_PARAM2 = "param2"

/**
 * A simple [Fragment] subclass.
 * Use the [MainFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class MainFragment : Fragment() {
    // TODO: Rename and change types of parameters
    private var param1: String? = null
    private var param2: String? = null

    private var cameraPermissionRequested = false

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
        // Inflate the layout for this fragment

        val view = inflater.inflate(R.layout.fragment_main, container, false)

        view.calibrate_camera_button.setOnClickListener { onCalibrateCameraClick(it) }
        view.show_depth_button.setOnClickListener { onShowDepthClick(it) }

        return view
    }

    private fun allPermissionsGranted() = CAMERA_REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(
            requireActivity().baseContext, it
        ) == PackageManager.PERMISSION_GRANTED
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<String>, grantResults:
        IntArray
    ) {
        if (requestCode == CAMERA_REQUEST_PERMISSIONS_CODE) {
            if (allPermissionsGranted()) {
                val action = MainFragmentDirections.actionMainFragmentToCalibrationFragment();
                calibrate_camera_button.findNavController().navigate(action);
            } else {
                Toast.makeText(
                    context,
                    "Camera permissions not granted",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }
    }


    private fun onCalibrateCameraClick(view: View) {
        if (allPermissionsGranted()) {
            val action = MainFragmentDirections.actionMainFragmentToCalibrationFragment();
            view.findNavController().navigate(action);
        } else {
            cameraPermissionRequested = true
            ActivityCompat.requestPermissions(
                requireActivity(), CAMERA_REQUIRED_PERMISSIONS, CAMERA_REQUEST_PERMISSIONS_CODE
            )
        }
    }

    private fun onShowDepthClick(view: View) {
        val action = MainFragmentDirections.actionMainFragmentToDepthFragment();
        view.findNavController().navigate(action);
    }

    companion object : Logger() {
        /**
         * Use this factory method to create a new instance of
         * this fragment using the provided parameters.
         *
         * @param param1 Parameter 1.
         * @param param2 Parameter 2.
         * @return A new instance of fragment MainFragment.
         */
        // TODO: Rename and change types and number of parameters
        @JvmStatic
        fun newInstance(param1: String, param2: String) =
            MainFragment().apply {
                arguments = Bundle().apply {
                    putString(ARG_PARAM1, param1)
                    putString(ARG_PARAM2, param2)
                }
            }

        private const val CAMERA_REQUEST_PERMISSIONS_CODE = 1
        private val CAMERA_REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
    }
}