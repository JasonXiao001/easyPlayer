package cn.jx.easyplayerlib.player;

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.widget.RelativeLayout;

import java.io.IOException;
import java.lang.ref.WeakReference;

import cn.jx.easyplayerlib.util.EasyLog;

/**
 *  Java wrapper of native easyPlayer.
 */

public class EasyMediaPlayer extends AbstractMediaPlayer {

    static {
        System.loadLibrary("player-lib");
    }

    private static final String TAG = EasyMediaPlayer.class.getSimpleName();

    private static final int MEDIA_NOP = 0; // interface test message
    private static final int MEDIA_PREPARED = 1;
    private static final int MEDIA_PLAYBACK_COMPLETE = 2;
    private static final int MEDIA_BUFFERING_UPDATE = 3;
    private static final int MEDIA_SEEK_COMPLETE = 4;
    private static final int MEDIA_SET_VIDEO_SIZE = 5;
    private static final int MEDIA_TIMED_TEXT = 99;
    private static final int MEDIA_ERROR = 100;
    private static final int MEDIA_INFO = 200;

    private boolean mScreenOnWhilePlaying;
    private boolean mStayAwake;
    private int mVideoWidth;
    private int mVideoHeight;
    private SurfaceHolder mSurfaceHolder;

    private Handler mHander = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what) {
                case MEDIA_SET_VIDEO_SIZE:
                    EasyLog.d(TAG, "on media size changed " + msg.arg1 + "*" + msg.arg2);
                    mVideoWidth = msg.arg1;
                    mVideoHeight = msg.arg2;
                    notifyOnVideoSizeChanged(mVideoWidth, mVideoHeight);
                    break;
                case MEDIA_PREPARED:
                    EasyLog.d(TAG, "on media prepared ");
                    notifyOnPrepared();

            }
            return true;
        }
    });


    private native void _nativeInit();

    public EasyMediaPlayer() {
        _nativeInit();
    }


    /**
     * Sets the data source (file-path or http/rtsp URL) to use.
     *
     * @param path
     *            the path of the file, or the http/rtsp URL of the stream you
     *            want to play
     * @throws IllegalStateException
     *             if it is called in an invalid state
     *
     *             <p>
     *             When <code>path</code> refers to a local file, the file may
     *             actually be opened by a process other than the calling
     *             application. This implies that the pathname should be an
     *             absolute path (as any other process runs with unspecified
     *             current working directory), and that the pathname should
     *             reference a world-readable file.
     */
    @Override
    public void setDataSource(String path) throws IOException, IllegalArgumentException, SecurityException, IllegalStateException {
        _setDataSource(path);
    }

    private native void _setDataSource(String path)
            throws IOException, IllegalArgumentException, SecurityException, IllegalStateException;

    /**
     * Sets the {@link SurfaceHolder} to use for displaying the video portion of
     * the media.
     *
     * Either a surface holder or surface must be set if a display or video sink
     * is needed. Not calling this method or {@link #setSurface(Surface)} when
     * playing back a video will result in only the audio track being played. A
     * null surface holder or surface will result in only the audio track being
     * played.
     *
     * @param sh
     *            the SurfaceHolder to use for video display
     */
    @Override
    public void setDisplay(SurfaceHolder sh) {
        mSurfaceHolder = sh;
        Surface surface;
        if (sh != null) {
            surface = sh.getSurface();
        } else {
            surface = null;
        }
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }


    /**
     * Sets the {@link Surface} to be used as the sink for the video portion of
     * the media. This is similar to {@link #setDisplay(SurfaceHolder)}, but
     * does not support {@link #setScreenOnWhilePlaying(boolean)}. Setting a
     * Surface will un-set any Surface or SurfaceHolder that was previously set.
     * A null surface will result in only the audio track being played.
     *
     * If the Surface sends frames to a {@link SurfaceTexture}, the timestamps
     * returned from {@link SurfaceTexture#getTimestamp()} will have an
     * unspecified zero point. These timestamps cannot be directly compared
     * between different media sources, different instances of the same media
     * source, or multiple runs of the same program. The timestamp is normally
     * monotonically increasing and is unaffected by time-of-day adjustments,
     * but it is reset when the position is set.
     *
     * @param surface
     *            The {@link Surface} to be used for the video portion of the
     *            media.
     */
    @Override
    public void setSurface(Surface surface) {
        if (mScreenOnWhilePlaying && surface != null) {
            EasyLog.w(TAG,
                    "setScreenOnWhilePlaying(true) is ineffective for Surface");
        }
        mSurfaceHolder = null;
        _setVideoSurface(surface);
        updateSurfaceScreenOn();
    }

    /*
     * Update the SurfaceTexture. Call after setting a new
     * display surface.
     */
    private native void _setVideoSurface(Surface surface);


    @Override
    public void prepareAsync() throws IllegalStateException {
        _prepareAsync();
    }

    @Override
    public void start() throws IllegalStateException {
        _start();
    }

    private native void _start() throws IllegalStateException;

    @Override
    public void stop() throws IllegalStateException {

    }

    @Override
    public void pause() throws IllegalStateException {
        _pause();
    }

    private native void _pause() throws IllegalStateException;

    @Override
    public int getVideoWidth() {
        return mVideoWidth;
    }

    @Override
    public int getVideoHeight() {
        return mVideoHeight;
    }

    @Override
    public native boolean isPlaying();

    @Override
    public native void seekTo(long msec) throws IllegalStateException;

    @Override
    public native long getCurrentPosition();

    @Override
    public native long getDuration();

    @Override
    public void release() {
//        stayAwake(false);
        updateSurfaceScreenOn();
        resetListeners();
        _release();
    }

    private native void _release();

    @Override
    public void reset() {

    }

    @Override
    public void setVolume(float leftVolume, float rightVolume) {

    }

    @Override
    public int getAudioSessionId() {
        return 0;
    }


    public native void _prepareAsync() throws IllegalStateException;


    @Override
    public void setScreenOnWhilePlaying(boolean screenOn) {
        if (mScreenOnWhilePlaying != screenOn) {
            if (screenOn && mSurfaceHolder == null) {
                EasyLog.w(TAG,
                        "setScreenOnWhilePlaying(true) is ineffective without a SurfaceHolder");
            }
            mScreenOnWhilePlaying = screenOn;
            updateSurfaceScreenOn();
        }
    }


    private void updateSurfaceScreenOn() {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.setKeepScreenOn(mScreenOnWhilePlaying && mStayAwake);
        }
    }



    private void postEventFromNative(int what, int arg1, int arg2) {
        EasyLog.i(TAG, "recv message from native,what " + what);
        Message.obtain(mHander, what, arg1, arg2).sendToTarget();
    }



}
