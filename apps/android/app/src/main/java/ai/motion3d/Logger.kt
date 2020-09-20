package ai.motion3d

import android.util.Log


open class Logger(var tag: String? = null) {
    init {
        if (tag == null) {
            if (this::class::isCompanion.get()) {
                val s: String? = this::class::qualifiedName.get()
                if (s != null) {
                    val parts = s.split('.')
                    tag = parts[parts.size - 2]
                }
            } else {
                tag = this::class.simpleName
            }
        }
    }

    fun V(msg: String, tr: Throwable? = null) = Log.v(tag, msg, tr)
    fun D(msg: String, tr: Throwable? = null) = Log.d(tag, msg, tr)
    fun I(msg: String, tr: Throwable? = null) = Log.i(tag, msg, tr)
    fun W(msg: String, tr: Throwable? = null) = Log.w(tag, msg, tr)
    fun E(msg: String, tr: Throwable? = null) = Log.e(tag, msg, tr)
}

