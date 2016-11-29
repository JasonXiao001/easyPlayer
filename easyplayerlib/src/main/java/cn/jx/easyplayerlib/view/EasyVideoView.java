package cn.jx.easyplayerlib.view;

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.view.Display;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.FrameLayout;
import android.widget.MediaController;
import android.widget.RelativeLayout;

import cn.jx.easyplayerlib.R;
import cn.jx.easyplayerlib.player.EasyMediaPlayer;
import cn.jx.easyplayerlib.player.IMediaPlayer;
import cn.jx.easyplayerlib.util.EasyLog;

/**
 *
 */

public class EasyVideoView extends FrameLayout implements MediaController.MediaPlayerControl, SurfaceHolder.Callback {

    private static final String TAG = EasyVideoView.class.getSimpleName();

    // all possible internal states
    private static final int STATE_ERROR = -1;
    private static final int STATE_IDLE = 0;
    private static final int STATE_PREPARING = 1;
    private static final int STATE_PREPARED = 2;
    private static final int STATE_PLAYING = 3;
    private static final int STATE_PAUSED = 4;
    private static final int STATE_PLAYBACK_COMPLETED = 5;

    private Context mAppContext;
    private IMediaPlayer mMediaPlayer = null;
    private MediaController mMediaController;
    private String mUri = null;
    private SurfaceHolder mSurfaceHolder;
    private SurfaceView mSurfaceView;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mSurfaceWidth;
    private int mSurfaceHeight;

    private int mCurrentState = STATE_IDLE;


    public EasyVideoView(Context context) {
        super(context);
        initVideoView(context);
    }

    public EasyVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initVideoView(context);
    }

    public EasyVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initVideoView(context);
    }


    private void initVideoView(Context context) {
        mAppContext = context.getApplicationContext();
        LayoutInflater.from(context).inflate(R.layout.view_easy_video, this);
        mSurfaceView = (SurfaceView) findViewById(R.id.surface_easy_video_view);
        mSurfaceView.getHolder().addCallback(this);
    }

    /**
     * Sets video path.
     *
     * @param path the path of the video.
     */
    public void setVideoPath(String path) {
        this.mUri = path;
        openVideo();
    }


    @Override
    public void start() {
        if (mMediaPlayer != null) {
            mMediaPlayer.start();
        }

    }

    @Override
    public void pause() {
        if (mMediaPlayer != null) {
            mMediaPlayer.pause();
        }

    }

    @Override
    public int getDuration() {
        if (mMediaPlayer != null && mCurrentState >= STATE_PREPARED) {
            return (int)mMediaPlayer.getDuration();
        }
        return -1;
    }

    @Override
    public int getCurrentPosition() {
        if (mMediaPlayer != null && mCurrentState >= STATE_PREPARED) {
            return (int)mMediaPlayer.getCurrentPosition();
        }
        return 0;
    }

    @Override
    public void seekTo(int pos) {
        if (mMediaPlayer != null && mCurrentState >= STATE_PREPARED) {
            mMediaPlayer.seekTo(pos);
        }
//        else {
//            mSeekWhenPrepared = msec;
//        }
    }

    @Override
    public boolean isPlaying() {
        return mMediaPlayer.isPlaying();
    }

    @Override
    public int getBufferPercentage() {
        return 0;
    }

    @Override
    public boolean canPause() {
        return true;
    }

    @Override
    public boolean canSeekBackward() {
        return true;
    }

    @Override
    public boolean canSeekForward() {
        return true;
    }

    /**
     * Get the audio session id for the player used by this VideoView. This can be used to
     * apply audio effects to the audio track of a video.
     *
     * @return The audio session, or 0 if there was an error.
     */
    @Override
    public int getAudioSessionId() {
        return 0;
    }

    /**
     * This is called immediately after the surface is first created.
     * Implementations of this should start up whatever rendering code
     * they desire.  Note that only one thread can ever draw into
     * a {@link Surface}, so you should not draw into the Surface here
     * if your normal rendering will be in another thread.
     *
     * @param holder The SurfaceHolder whose surface is being created.
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mSurfaceHolder = holder;
        if (mMediaPlayer == null) {
            openVideo();
        }
    }

    /**
     * This is called immediately after any structural changes (format or
     * size) have been made to the surface.  You should at this point update
     * the imagery in the surface.  This method is always called at least
     * once, after {@link #surfaceCreated}.
     *
     * @param holder The SurfaceHolder whose surface has changed.
     * @param format The new PixelFormat of the surface.
     * @param width  The new width of the surface.
     * @param height The new height of the surface.
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    /**
     * This is called immediately before a surface is being destroyed. After
     * returning from this call, you should no longer try to access this
     * surface.  If you have a rendering thread that directly accesses
     * the surface, you must ensure that thread is no longer touching the
     * Surface before returning from this function.
     *
     * @param holder The SurfaceHolder whose surface is being destroyed.
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }




    private void openVideo() {
        if (mUri == null || mSurfaceHolder == null) {
            return;
        }
        EasyLog.i(TAG, "video view is ready,creating player");
        mMediaPlayer = new EasyMediaPlayer();
        mMediaPlayer.setOnVideoSizeChangedListener(mSizeChangedListener);
        mMediaPlayer.setOnPreparedListener(mPreparedListener);
        try {
            mMediaPlayer.setDataSource(mUri);
            mMediaPlayer.setSurface(mSurfaceHolder.getSurface());
            new Thread(new Runnable() {
                @Override
                public void run() {
                    mMediaPlayer.prepareAsync();
                    mCurrentState = STATE_PREPARING;
                }
            }).start();
            if (mMediaController == null) {
                mMediaController = new MediaController(getContext());
                mMediaController.setMediaPlayer(this);
                mMediaController.setAnchorView(this);
                mMediaController.setEnabled(true);
                mMediaController.show();
            }

        }catch (Exception e) {

        }

    }


    private boolean isInPlaybackState() {
        return (mMediaPlayer != null &&
                mCurrentState != STATE_ERROR &&
                mCurrentState != STATE_IDLE &&
                mCurrentState != STATE_PREPARING);
    }

    private void toggleMediaControlsVisiblity() {
        if (mMediaController.isShowing()) {
            mMediaController.hide();
        } else {
            mMediaController.show();
        }
    }




    IMediaPlayer.OnVideoSizeChangedListener mSizeChangedListener =
            new IMediaPlayer.OnVideoSizeChangedListener() {
                public void onVideoSizeChanged(IMediaPlayer mp, int width, int height) {
                    mVideoWidth = mp.getVideoWidth();
                    mVideoHeight = mp.getVideoHeight();
//                    mSurfaceHolder.setFixedSize(mVideoWidth, mVideoHeight);
//                    requestLayout();
                    Display display = getDisplay();
                    final int displayWidth = display.getWidth();
                    if (mVideoWidth != 0 && mVideoHeight != 0) {
                        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) getLayoutParams();
                        params.height = displayWidth*mVideoHeight/mVideoWidth;
                        setLayoutParams(params);
                    }
                }
            };


    IMediaPlayer.OnPreparedListener mPreparedListener = new IMediaPlayer.OnPreparedListener() {
        public void onPrepared(IMediaPlayer mp) {
            mCurrentState = STATE_PREPARED;
        }
    };
}
