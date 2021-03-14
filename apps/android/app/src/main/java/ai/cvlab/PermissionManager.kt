package ai.cvlab

import android.Manifest
import android.content.pm.PackageManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.util.*
import kotlin.collections.HashMap

fun interface PermissionSubscriber {
    fun onPermissionResult(granted: Boolean);
}

object PermissionManager : Logger() {
    private const val CAMERA_REQUEST_PERMISSIONS_CODE = 1
    private val CAMERA_REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)

    private var activity: AppCompatActivity? = null;
    private val subscribers: HashMap<Int, LinkedList<PermissionSubscriber>> = HashMap()

    private fun addSubscriber(requestCode: Int, subscriber: PermissionSubscriber) {
        if (!subscribers.containsKey(requestCode)) {
            subscribers[requestCode] = LinkedList()
        }
        subscribers[requestCode]!!.add(subscriber)
    }

    fun setActivity(activity: AppCompatActivity?) {
        this.activity = activity
    }

    fun requestCamera(subscriber: PermissionSubscriber) {
        if (activity == null) {
            W("requestCamera. Subscriber: $subscriber. Activity is not set")
            subscriber.onPermissionResult(false);
            return;
        }
        if (cameraPermissionsGranted()) {
            subscriber.onPermissionResult(true)
            return;
        }

        addSubscriber(CAMERA_REQUEST_PERMISSIONS_CODE, subscriber)
        ActivityCompat.requestPermissions(
            activity!!, CAMERA_REQUIRED_PERMISSIONS, CAMERA_REQUEST_PERMISSIONS_CODE
        )
    }

    private fun cameraPermissionsGranted(): Boolean {
        if (activity == null) {
            W("cameraPermissionsGranted. Activity is not set")
            return false
        }

        return CAMERA_REQUIRED_PERMISSIONS.all {
            ContextCompat.checkSelfPermission(
                activity!!.baseContext, it
            ) == PackageManager.PERMISSION_GRANTED
        }
    }

    fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        D("onRequestPermissionsResult. requestCode = $requestCode. permissions = ${permissions.str()}, grantResults = ${grantResults.str()}")
        if (requestCode == CAMERA_REQUEST_PERMISSIONS_CODE) {
            val granted = cameraPermissionsGranted()
            I("Camera permissions granted: $granted")
            val subs = subscribers.remove(CAMERA_REQUEST_PERMISSIONS_CODE);
            if (subs != null) {
                for (sub in subs) {
                    sub.onPermissionResult(granted);
                }
            }
            if (!granted && activity != null) {
                Toast.makeText(
                    activity,
                    "Camera permissions not granted",
                    Toast.LENGTH_SHORT
                ).show()
            }
        }
    }

}

