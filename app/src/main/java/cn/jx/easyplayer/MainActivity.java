package cn.jx.easyplayer;


import android.app.Activity;
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
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


public class MainActivity extends Activity{

    private static final String TAG = MainActivity.class.getSimpleName();

    private SurfaceHolder surfaceViewHolder;
    private SurfaceView surfaceView;
    private Handler mainHandler = new Handler();



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final EasyVideoView videoView = (EasyVideoView) findViewById(R.id.easy_video_view);
        String folderurl = Environment.getExternalStorageDirectory().getPath();
        String inputurl = folderurl+"/StarCraft.mp4";
        videoView.setVideoPath(inputurl);
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


}
