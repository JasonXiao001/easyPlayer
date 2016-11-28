package cn.jx.easyplayer;

import android.os.Handler;
import android.util.Log;
import android.view.Surface;



/**
 *
 */

public class EasyPlayer {

    static final String TAG = EasyPlayer.class.getSimpleName();

//    private VideoEventListener videoEventListener;
    private Handler mainHandler = new Handler();


    static {
        System.loadLibrary("native-lib");
    }


    public native void play(String url, Surface surface);

    private void onResolutionChange(final int width,final int height){
        Log.d(TAG, "height: "+height+" width:"+width);
//        if (videoEventListener != null) {
//            videoEventListener.onResolutionChange(width, height);
//        }


    }

//    public void setVideoEventListener(VideoEventListener videoEventListener) {
//        this.videoEventListener = videoEventListener;
//    }
}
