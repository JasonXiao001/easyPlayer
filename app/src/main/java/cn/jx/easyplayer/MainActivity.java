package cn.jx.easyplayer;


import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;

import cn.jx.easyplayerlib.EasyPlayer;
import cn.jx.easyplayerlib.events.VideoEventListener;
import cn.jx.easyplayerlib.player.EasyMediaPlayer;
import cn.jx.easyplayerlib.view.EasyVideoView;


public class MainActivity extends AppCompatActivity{

    private static final String TAG = MainActivity.class.getSimpleName();

    private SurfaceHolder surfaceViewHolder;
    private SurfaceView surfaceView;
    private Handler mainHandler = new Handler();
//    EasyPlayer easyPlayer = new EasyPlayer();



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final EasyVideoView videoView = (EasyVideoView) findViewById(R.id.easy_video_view);
        String folderurl = Environment.getExternalStorageDirectory().getPath();
        String inputurl = folderurl+"/jack.mp4";
        videoView.setVideoPath(inputurl);
//        surfaceView = (SurfaceView) findViewById(R.id.video_view);
//        surfaceViewHolder = surfaceView.getHolder();
//        surfaceViewHolder.addCallback(new SurfaceHolder.Callback() {
//            @Override
//            public void surfaceCreated(SurfaceHolder holder) {
//                new Thread(new Play()).start();
//            }
//
//            @Override
//            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
//
//            }
//
//            @Override
//            public void surfaceDestroyed(SurfaceHolder holder) {
//
//            }
//        });
        Button pause = (Button) findViewById(R.id.pause);
        pause.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                videoView.start();

            }
        });



    }

    @Override
    protected void onPause() {
        super.onPause();
        togglePaused();
    }



    class Play implements Runnable {

        @Override
        public void run() {
            String folderurl = Environment.getExternalStorageDirectory().getPath();
            String inputurl = folderurl+"/jack.mp4";
//            String inputurl = "http://200000291.vod.myqcloud.com/200000291_5bdb30893e5848188f9f8d29c24b1fa6.f0.mp4";
//            String inputurl = "http://106.36.45.36/live.aishang.ctlcdn.com/00000110240001_1/encoder/1/playlist.m3u8";
//            String inputurl = "http://1251659802.vod2.myqcloud.com/vod1251659802/9031868222807497694/f0.mp4";
//            String inputurl = "rtmp://2107.liveplay.myqcloud.com/live/2107_3100673b756411e69776e435c87f075e";
//            play(inputurl, surfaceViewHolder.getSurface());
//            easyPlayer.play(inputurl, surfaceViewHolder.getSurface());
        }
    }


    public void onResolutionChange(final int width,final int height){
        Display display = getWindowManager().getDefaultDisplay();
        final int displayWidth = display.getWidth();
        mainHandler.post(new Runnable(){

            @Override
            public void run() {
                // TODO Auto-generated method stub
                RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) surfaceView.getLayoutParams();
                params.height = displayWidth*height/width;
                surfaceView.setLayoutParams(params);
            }

        });


    }


    public native void togglePaused();



}
