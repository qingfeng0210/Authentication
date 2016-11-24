package cn.dacas.authentication.ui;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import cn.dacas.authentication.R;
import cn.dacas.authentication.model.NativeCode;
import cn.dacas.authentication.util.HttpUtil;
import cn.dacas.authentication.zxing.ui.MipcaActivityCapture;
import cn.dcs.security.sm.Sm2;

import static cn.dacas.authentication.util.HttpUtil.getRequest;

/**
 * Created by qingf on 2016/9/22.
 */
public class MainActivity extends Activity {
    public static final String SCAN_URL = HttpUtil.BASE_URL+"/qr/query/";

    private static final String LOG_OUT_URL = HttpUtil.BASE_URL+"/service/logout";
    public static final String COOKIE_URL = HttpUtil.BASE_URL+"/service/flush?sn=";
    private final static int SCANNIN_GREQUEST_CODE = 0x00c1;
    private static final String TAG = "PostID";
    private final int COOKIE_WHAT =0x1233;
    private Context mContext;

    private Button scan_bt;
    private Button lg_out;
    private static String result = "";
    private static int test_flag = 0;
    private String password;

    private Sm2 sm2;

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext=this;
        password=getIntent().getExtras().getString("password");
        setContentView(R.layout.activity_main);
        scan_bt = (Button) findViewById(R.id.scan_bt);
        lg_out = (Button) findViewById(R.id.log_out);
        scan_bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.d(TAG, "onClick: scan_bt");
                Intent intent = new Intent(mContext,MipcaActivityCapture.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                startActivityForResult(intent, SCANNIN_GREQUEST_CODE);
                Log.d(TAG, "onClick: scan_bt");
            }
        });
        lg_out.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    String strResult = getRequest(LOG_OUT_URL);
                    Log.d(TAG,strResult);

                    Intent intent = new Intent(mContext,LoginActivity.class);
                    startActivity(intent);

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        final Handler myHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                if(msg.what == COOKIE_WHAT) {
                    Log.d(TAG, "handleMessage: "+msg+"test_flag: "+test_flag);
                }
            }
        };
        /*
        new Timer().schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    String curResult = HttpUtil.getRequest(COOKIE_URL);
                    JSONArray jsonArray = new JSONArray(curResult);
                    for(int i=0;i<jsonArray.length();i++) {
                        JSONObject object = (JSONObject) jsonArray.getJSONObject(i);

                    }
                    ++test_flag;
                    if(test_flag%5==1) {
                        myHandler.sendEmptyMessage(COOKIE_WHAT);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        },0,1200);
        */
        Button bt_key = (Button) findViewById(R.id.generate_key);
        bt_key.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Sm2 sm2 = new Sm2();
                TextView textview = (TextView) findViewById(R.id.view_key);
                NativeCode.tryTest();
                Log.d(TAG, "onClick: "+"second true");
                Log.d(TAG, "onClick: key====begin");


                byte []pin=new byte[32];
                int iHash=NativeCode.sm3Hash(password,pin);
                Log.d(TAG, "onClick: iHash: "+iHash);
                int iRet = NativeCode.generateUserKeyPair(pin);
                Log.d(TAG, "iRet: "+iRet);


                /*
                try {
                    Log.d(TAG, "onClick: public "+Util.bytes2HexString(pub));
                    byte [] prv = sm2.getPrivateKey();
                    Log.d(TAG, "onClick: private "+Util.bytes2HexString(prv));
                    byte [] plaintext = "test123456".getBytes();
                    Log.d(TAG, "onClick: plain="+Util.bytes2HexString(plaintext));
                    byte[] text = sm2.encrypt(plaintext);
                    Log.d(TAG, "onClick: text=:"+Util.bytes2HexString(text));
                    byte [] plaintext2 = sm2.decrypt(text);
                    Log.d(TAG, "onClick: plain2="+ Util.bytes2HexString(plaintext2));
                    byte []sign = sm2.sign(plaintext);
                    Log.d(TAG, "onClick: sign"+Util.bytes2HexString(sign));
                    Log.d(TAG, "onClick: "+sm2.verify(plaintext,sign));

                } catch (IOException e) {
                    e.printStackTrace();
                } catch (OpException e) {
                    e.printStackTrace();
                }*/
            }
        });

    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case SCANNIN_GREQUEST_CODE:
                if(resultCode == RESULT_OK){
                    Bundle bundle = data.getExtras();
                    result = bundle.getString("result");
                    Log.d(TAG, "onActivityResult: "+result);
                    String resultReq =null;
                    try {
                        resultReq = HttpUtil.getRequest(SCAN_URL+result);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    if(resultReq==null) {
                        Intent intentFalse = new Intent(mContext,ErrorActivity.class);
                        startActivity(intentFalse);
                    }
                    else{
                        Intent intentTrue =new Intent(mContext,ConfirmActivity.class);
                        intentTrue.putExtra("id_rand",result);
                        startActivity(intentTrue);
                    }
                }
                break;
        }
    }


    //
    private static byte[] hexStringToBytes(String hexString) {   //可以删除
        if (hexString == null || hexString.equals("")) {
            return null;
        }
        hexString = hexString.toUpperCase();
        int length = hexString.length() / 2;
        char[] hexChars = hexString.toCharArray();
        byte[] d = new byte[length];
        for (int i = 0; i < length; i++) {
            int pos = i * 2;
            d[i] = (byte) (charToByte(hexChars[pos]) << 4 | charToByte(hexChars[pos + 1]));
        }
        return d;
    }
    private static byte charToByte(char c) {   //可以删除
        return (byte) "0123456789ABCDEF".indexOf(c);
    }

}
