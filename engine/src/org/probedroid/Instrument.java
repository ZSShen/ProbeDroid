/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

package org.probedroid;

import org.probedroid.support.MethodBundle;

/**
 * The entry point of ProbeDroid which defines the callbacks indicating the
 * change of app life cycle and also provides the interfaces for analysts to
 * register the instrumentation gadgets for interested methods. <b><i>Analysts
 * should inherit this class as the main class of their analysis
 * package.</i></b>
 *
 * <pre class="prettyprint">
 * public class MyInstrument extends Instrument {
 *
 *     &#064;Override
 *     public void onApplicationStart() {
 *         // Do some initialization.
 *
 *         // Register the instrumentation gadgets.
 *         String nameClass = &quot;java.lang.StringBuilder&quot;;
 *         String nameMethod = &quot;toString&quot;;
 *         String signatureMethod = &quot;()Ljava/lang/String;&quot;;
 *         BundleStringBuilder bundleBuilder = new BundleStringBuilder(false, true);
 *         instrumentMethod(false, nameClass, nameMethod, signatureMethod,
 *                 bundleBuilder);
 *     }
 *
 *     &#064;Override
 *     public void onApplicationStop() {
 *         // Do some finalization.
 *     }
 * }
 * </pre>
 */
public abstract class Instrument {

    public static String mPathOutputDirectory;

    /**
     * @hide
     */
    public void prepareNativeBridge(String pathLib) {
        System.load(pathLib);
    }

    /**
     * Let analysts to <b><i>register instrument gadget for the interested
     * method.</i></b> Currently, the abstract method or native method is not
     * supported yet.
     *
     * @param isStatic
     *            The flag indicating whether the target method is static or
     *            not.
     * @param nameClass
     *            The name of the class defining the target method.
     * @param nameMethod
     *            The target method name.
     * @param signatureMethod
     *            The target method signature representing the data types of
     *            arguments and return value.
     * @param bundle
     *            The instrument gadget for the target method.
     *
     *
     * @throws ClassNotFoundException
     *             The class defining the target method cannot be found.
     *
     * @throws NoSuchMethodException
     *             The target method cannot be resolved (Probably due to wrong
     *             signature).
     * @throws IllegalArgumentException
     *             Illegal arguments passed to this method (Probably null String
     *             or MethodBundle).
     *
     */
    public void instrumentMethod(boolean isStatic, String nameClass,
            String nameMethod, String signatureMethod, MethodBundle bundle)
            throws ClassNotFoundException, NoSuchMethodException,
            IllegalArgumentException {
        instrumentMethodNative(isStatic, nameClass, nameMethod,
                signatureMethod, bundle);
    }

    /**
     * The callback indicating that the instrumented app is just launched.
     * <b><i>Analysts should override this method for initialization.</i></b>
     * Like registering the instrumentation gadgets or creating the custom
     * profiling utilities.
     */
    public abstract void onApplicationStart();

    /**
     * The callback indicating that the instrumented app is just terminated.
     * <b><i>Analysts should override this method for finalization.</i></b> Like
     * dumping the profiled information to the external storage.
     */
    public abstract void onApplicationStop();

    private native void instrumentMethodNative(boolean isStatic,
            String nameClass, String nameMethod, String signatureMethod,
            MethodBundle bundle) throws ClassNotFoundException,
            NoSuchMethodException, IllegalArgumentException;
}
