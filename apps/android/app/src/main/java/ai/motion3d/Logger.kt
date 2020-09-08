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

    fun lv(msg: String, tr: Throwable? = null) = Log.v(tag, msg, tr)
    fun ld(msg: String, tr: Throwable? = null) = Log.d(tag, msg, tr)
    fun li(msg: String, tr: Throwable? = null) = Log.i(tag, msg, tr)
    fun lw(msg: String, tr: Throwable? = null) = Log.w(tag, msg, tr)
    fun le(msg: String, tr: Throwable? = null) = Log.e(tag, msg, tr)
}

