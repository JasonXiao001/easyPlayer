package cn.jx.easyplayer;


import android.content.Intent;
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
import android.widget.RelativeLayout;

import cn.jx.easyplayerlib.EasyPlayer;
import cn.jx.easyplayerlib.view.VideoView;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();

    private SurfaceHolder surfaceViewHolder;
    private SurfaceView surfaceView;
    private Handler mainHandler = new Handler();
//    EasyPlayer easyPlayer = new EasyPlayer();
//    VideoView videoView;


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = (SurfaceView) findViewById(R.id.video_view);
        surfaceViewHolder = surfaceView.getHolder();
        surfaceViewHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                new Thread(new Play()).start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });



    }

    class Play implements Runnable {

        @Override
        public void run() {
            String folderurl = Environment.getExternalStorageDirectory().getPath();
            String inputurl = folderurl+"/jack.mp4";
//            String inputurl = "http://200000291.vod.myqcloud.com/200000291_5bdb30893e5848188f9f8d29c24b1fa6.f0.mp4";
//            String inputurl = "http://1251659802.vod2.myqcloud.com/vod1251659802/9031868222807497694/f0.mp4";
            play(inputurl, surfaceViewHolder.getSurface());
        }
    }

    private void onResolutionChange(final int width,final int height){
        Log.d(TAG, "height: "+height+" width:"+width);
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

    public native int play(String url, Surface surface);

}
