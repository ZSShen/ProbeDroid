package org.probedroid.support;

/**
 * The interface of method instrumentation gadget. <b><i>Analysts should inherit
 * this class to craft their own instrumentation logic.</i></b> For example,
 * determining how to utilize the intercepted input arguments or the return
 * value.
 *
 * <pre class="prettyprint">
 * public class MyMethodBundle extends MethodBundle {
 *     public MyMethodBundle(boolean interceptBefore,
 *             boolean interceptAfter) {
 *         super(interceptBefore, interceptAfter);
 *     }
 *
 *     &#064;Override
 *     public void beforeMethodExecute(Object[] objects) {
 *         // Determine how to utilize the intercepted input arguments.
 *     }
 *
 *     &#064;Override
 *     public void afterMethodExecute(Object o) {
 *         // Determine how to utilize the intercepted return value if any.
 *     }
 * }
 * </pre>
 */
public abstract class MethodBundle {
    private final boolean mInterceptBefore;
    private final boolean mInterceptAfter;

    /**
     * @param interceptBefore
     *            The flag indicating whether to intercept the target method
     *            when it is called but not executed yet.
     * @param interceptAfter
     *            The flag indicating whether to intercept the target method
     *            when it is executed but not returned yet.
     */
    public MethodBundle(boolean interceptBefore, boolean interceptAfter) {
        mInterceptBefore = interceptBefore;
        mInterceptAfter = interceptAfter;
    }

    /**
     * The callback to intercept the input arguments before the target method is
     * executed. <b><i>Note that all the arguments are boxed as java.lang.Object
     * type</i></b>. Analyst can cast each boxed object to its original type for
     * manipulation. Also, modifying the argument content or replacing the
     * argument with the one instantiated from this callback are both allowed.
     * Therefore, simple data profiling and dynamic behavior modification are
     * possible.
     *
     * <pre class="prettyprint">
     * // Suppose we plan to instrument StringBuffer StringBuffer.append(String)
     * &#064;Override
     * public void beforeMethodExecute(Object[] objects) {
     *     String str = (String) objects[0];
     *     Log.d(&quot;My Instrument&quot;, str);
     *     objects[0] = new String(&quot;Hack&quot;);
     * }
     * </pre>
     *
     * @param inputs
     *            The array of boxed input arguments of the target method.
     */
    public abstract void beforeMethodExecute(Object[] inputs);

    /**
     * The callback to intercept the return value after the target method is
     * executed. <b><i>Note that the return value is boxed as java.lang.Object
     * type</i></b>. Analyst can cast the boxed object to its original type for
     * manipulation. Also, modifying the value content or replacing the value
     * with the one instantiated from this callback are both allowed. Therefore,
     * simple data profiling and dynamic behavior modification are possible.
     *
     * <pre class="prettyprint">
     * // Suppose we plan to instrument StringBuffer StringBuffer.append(String)
     * &#064;Override
     * public void afterMethodExecute(Object object) {
     *     StringBuffer sb = (StringBuffer) object;
     *     sb.append(&quot; Hack Again&quot;);
     * }
     * </pre>
     *
     * @param output
     *            The boxed return value of the target method or null if the
     *            target method returns void.
     */
    public abstract void afterMethodExecute(Object output);
}
