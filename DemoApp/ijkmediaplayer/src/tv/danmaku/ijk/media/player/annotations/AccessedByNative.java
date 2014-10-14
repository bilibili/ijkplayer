package tv.danmaku.ijk.media.player.annotations;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * @CalledByNative is used by the JNI generator to create the necessary JNI
 *                 bindings and expose this method to native code.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.CLASS)
public @interface AccessedByNative {
}