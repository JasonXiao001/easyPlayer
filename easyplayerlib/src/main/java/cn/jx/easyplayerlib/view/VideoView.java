package cn.jx.easyplayerlib.view;

import android.app.Activity;
import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.RelativeLayout;

import cn.jx.easyplayerlib.EasyPlayer;
import cn.jx.easyplayerlib.R;
import cn.jx.easyplayerlib.events.VideoEventListener;

/**
 *
 */

public class VideoView  extends RelativeLayout {

    static private final String TAG = VideoView.class.getSimpleName();
    private SurfaceHolder surfaceViewHolder;
    private SurfaceView surfaceView;
    private boolean surfaceReady = false;
    private boolean start = false;
    private EasyPlayer player = new EasyPlayer();
    private String url;

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater.from(context).inflate(R.layout.video_view, this);
        surfaceView = (SurfaceView) findViewById(R.id.surface);
        surfaceViewHolder = surfaceView.getHolder();
        surfaceViewHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                surfaceReady = true;
                if (start) {
                    new PlayThread().run();
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.d(TAG, "width " + width + " height " + height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
        player.setVideoEventListener(new VideoEventListener() {
            @Override
            public void onResolutionChange(int width, int height) {
                Display display = ((Activity) VideoView.this.getContext()).getWindowManager().getDefaultDisplay();
                final int displayWidth = display.getWidth();
                RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) surfaceView.getLayoutParams();
                params.height = displayWidth*height/width;
                surfaceView.setLayoutParams(params);
            }
        });
    }

    public void play(String url) {
        if (surfaceReady) {
            new PlayThread().run();
        }else {
            start = true;
            this.url = url;
        }
    }

    class PlayThread implements Runnable {

        @Override
        public void run() {
            Surface surface = surfaceViewHolder.getSurface();
            player.play(url, surface);
        }
    }



}
