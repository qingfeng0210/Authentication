package cn.dacas.authentication.ui;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.apache.http.client.ClientProtocolException;

import java.io.IOException;
import java.io.UnsupportedEncodingException;

import cn.dacas.authentication.R;
import cn.dacas.authentication.util.HttpUtil;

/**
 * Created by qingf on 2016/9/30.
 */

public class ConfirmActivity extends Activity implements View.OnClickListener {

    public static final String CONFIRM_URL = HttpUtil.BASE_URL+"/qr/confirm/";
    public static final String CANCLE_URL = HttpUtil.BASE_URL+"/qr/cancel/";
    private Button btConfirm;
    private Button btCancle;
    private String confirm_cancel = "cancel";
    private TextView close_view;
    private String id_rand;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();
        id_rand = intent.getStringExtra("id_rand");

        Log.d("PostID","after_id_rand: "+id_rand);
        setContentView(R.layout.activity_confirm);
        close_view = (TextView) findViewById(R.id.close_view);
        btConfirm = (Button) findViewById(R.id.confirm_login);
        btCancle = (Button) findViewById(R.id.cancel_login);
        close_view.setOnClickListener(ConfirmActivity.this);
        btConfirm.setOnClickListener(ConfirmActivity.this);
        btCancle.setOnClickListener(ConfirmActivity.this);
    }


    @Override
    public void onClick(View view) {
        String result;
        switch (view.getId()){
            case R.id.confirm_login:

                result = doPost(CONFIRM_URL);
                if(result== null){
                    Toast.makeText(this,"允许登录失败！", Toast.LENGTH_SHORT).show();
                    Log.d("PostID", "receive error1");
                    Intent intentError = new Intent(ConfirmActivity.this,ErrorActivity.class);
                    startActivity(intentError);
                }
                else {
                    Log.d("PostID","login true");
                    Toast.makeText(this,"允许登录成功！", Toast.LENGTH_SHORT).show();
                    Intent intentMain = new Intent(ConfirmActivity.this,MainActivity.class);
                    startActivity(intentMain);
                }
                break;
            case R.id.close_view:
                Intent intentClose = new Intent(ConfirmActivity.this,MainActivity.class);
                startActivity(intentClose);
                break;
            case R.id.cancel_login:

                result = doPost(CANCLE_URL);
                if(result==null) {
                    Toast.makeText(this,"取消登录失败！", Toast.LENGTH_SHORT).show();
                    Log.d("PostID", "receive error2");
                    Intent intentError = new Intent(ConfirmActivity.this,ErrorActivity.class);
                    startActivity(intentError);
                }
                else {
                    Log.d("PostID", "login false");
                    Toast.makeText(this, "取消登录成功！", Toast.LENGTH_SHORT).show();
                    Intent intentMain = new Intent(ConfirmActivity.this, MainActivity.class);
                    startActivity(intentMain);
                }
                break;
            default:
                break;
        }
    }

    /**
     * 用Post方式跟服务器传递数据
     * @param url
     * @return
     */

    private String doPost(String url){
        String result = "";
        try {
            //下面开始跟服务器传递数据，使用BasicNameValuePair
            //Map<String ,String> map = new HashMap<String, String>();
            //map.put("id_rand",id_rand);
            //JSONObject jsonObject = new JSONObject(map);
            result = HttpUtil.postRequest(url+id_rand,null);
            if(result==null)
                Log.d("PostID","result is null");
            else
                Log.d("PostID",result);
        } catch (UnsupportedEncodingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (ClientProtocolException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }
}
