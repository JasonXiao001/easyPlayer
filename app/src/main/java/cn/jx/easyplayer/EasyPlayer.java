package cn.jx.easyplayer;

import android.view.Surface;

/**
 * @author jasonxiao{github.com/JasonXiao001}
 */
public class EasyPlayer {

    static {
        System.loadLibrary("native-lib");
    }

    public void setDataSource(String url) {
        _setDataSource(url);
    }

    public void setEventCallback(EasyPlayerEventCallback cb) {
        _setEventCallback(cb);
    }

    public void prepareAsync() {
        _prepareAsync();
    }

    public void start() {
        _start();
    }

    public void pause() {
        _pause();
    }

    public void setSurface(Surface surface) {
        _setSurface(surface);
    }

    public int getVideoWidth() {
        return _getVideoWidth();
    }

    public int getVideoHeight() {
        return _getVideoHeight();
    }

    private native void _setEventCallback(EasyPlayerEventCallback cb);

    private native void _setDataSource(String url);

    private native void _prepareAsync();

    private native void _start();

    private native void _pause();

    private native void _setSurface(Surface surface);

    private native int _getVideoWidth();

    private native int _getVideoHeight();
}
