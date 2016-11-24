package cn.dacas.authentication.ui;

import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TableRow;

import org.json.JSONException;
import org.json.JSONObject;

import cn.dacas.authentication.R;
import cn.dacas.authentication.model.NativeCode;
import cn.dacas.authentication.util.DialogUtil;
import cn.dacas.authentication.util.HttpUtil;


/**
 * Created by qingf on 2016/11/2.
 */

public class LoginActivity extends Activity {
    public static final String URL = HttpUtil.BASE_URL+"/service/login";
    public static final String CODE_URL = HttpUtil.BASE_URL+"/code";
    private static final String TAG = "PostID";
    private EditText userText = null;
    private EditText pwdText = null;
    private CheckBox rememberPwd;
    private CheckBox showPwd;
    private ImageView checkImage;
    private EditText codeText;
    private Button buttonCheckLogin = null;
    private TableRow codeRow;

    private SharedPreferences pref;
    private SharedPreferences.Editor editor;
    private boolean codeLogin = false;
    private String statusLogin = "";
    private String cookie_sn = "0";
    private String name;
    private String password;
    static {
        System.loadLibrary("native-lib");
    }
    public static native String stringFromJNI();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        Log.d(TAG, "onCreate: "+stringFromJNI());

        NativeCode.tryTest();
        Log.d(TAG, "onCreate: "+"true");

        initView();
        Log.d(TAG,"begin");
        showPwd.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(showPwd.isChecked()){//当showPwd被选中，密文以明文形式显示
                    pwdText.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                }
                else{//不选中，以密文形式显示
                    pwdText.setTransformationMethod(PasswordTransformationMethod.getInstance());
                }
            }
        });
        checkImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    Bitmap bitmap = HttpUtil.getRequestBitmap(CODE_URL);
                    checkImage.setImageBitmap(bitmap);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        boolean isRemember = pref.getBoolean("rememberPassword",false);
        if(isRemember){
            //将账号和密码都设置到文本框中
            name = pref.getString("user","");
            password = pref.getString("password","");
            userText.setText(name);
            pwdText.setText(password);
            rememberPwd.setChecked(true);

            //不需输入密码，直接发送给服务器用户的公钥即可

        }
        buttonCheckLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //下面开始跟服务器传递数据，使用BasicNameValuePair
                name = userText.getText().toString().trim();
                password = pwdText.getText().toString().trim();
                String postRes = "";
                String raw = "user="+name+"&password="+password;
                if(codeLogin) {
                    String code = codeText.getText().toString().trim();
                    Log.d(TAG,"code: "+code);
                    raw+="&code="+ code;
                }
                try {
                    postRes = HttpUtil.postRequest(URL,raw);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                if(postRes!=null){
                    Log.d(TAG,"postRes: "+postRes);
                    postRes = postRes.trim();
                    try {
                        JSONObject jsonObject = new JSONObject(postRes);
                        statusLogin = jsonObject.getString("result");
                        if (statusLogin.equals("0")) {//{"result":0, "user":"user2"}
                            Log.d(TAG, "正常登录");
                            if(rememberPwd.isChecked()){

                                //此处不需要存储密码，保存用户的公钥

                                editor.putBoolean("rememberPassword",true);
                                editor.putString("user",name);
                                editor.putString("password",password);
                            }
                            else{
                                editor.clear();
                            }
                            editor.commit();
                            //startFlushCookie(COOKIE_URL);
                            Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                            intent.putExtra("password",password);
                            startActivity(intent);
                            //this.finish();
                        } else if (statusLogin.equals("1")) {//{"result":1,"error":"用户名口令错误"}
                            String error = jsonObject.getString("error");
                            DialogUtil.showDialog(LoginActivity.this
                                    , error, false);
                            Log.d(TAG, error);
                        } else if (codeLogin||statusLogin.equals("2")) {//{"result":2,"error":"请输入验证码"}
                            codeLogin = true;
                            String error = jsonObject.getString("error");
                            DialogUtil.showDialog(LoginActivity.this
                                    , error, false);
                            Log.d(TAG, error);
                            //显示并更新验证码
                            codeRow.setVisibility(View.VISIBLE);
                            try {
                                Bitmap bitmap = HttpUtil.getRequestBitmap(CODE_URL);
                                checkImage.setImageBitmap(bitmap);
                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        } else if (statusLogin.equals("3")) {//请选择单位，返回[{"code":"dacas.cn","name":""},…]
                            String error = jsonObject.getString("error");
                            DialogUtil.showDialog(LoginActivity.this
                                    , error, false);
                            Log.d(TAG, error);
                        } else if (statusLogin.equals("4")) {//用户登录失败10次，请5分钟后再登录
                            String error = jsonObject.getString("error");
                            DialogUtil.showDialog(LoginActivity.this
                                    , error, false);
                            Log.d(TAG, error);
                            //buttonCheckLogin.setEnabled(false);
                        }
                    }catch (JSONException e) {
                        e.printStackTrace();
                    }
                }

            }
         });
    }
    private void initView() {
        userText = (EditText) findViewById(R.id.edit_user);
        pwdText = (EditText) findViewById(R.id.edit_password);
        rememberPwd = (CheckBox) findViewById(R.id.remember_pwd);
        showPwd = (CheckBox) findViewById(R.id.show_pwd);
        codeRow = (TableRow) findViewById(R.id.code_row);
        checkImage = (ImageView) findViewById(R.id.check_image);
        codeText = (EditText) findViewById(R.id.edit_checkcode);
        buttonCheckLogin = (Button) findViewById(R.id.bnCheckLogin);
        //密码状态
        pwdText.setInputType(EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_TEXT_VARIATION_PASSWORD);
        pref = getSharedPreferences("data",MODE_PRIVATE);
        editor = pref.edit();
    }
    private void startFlushCookie(String url) {
        try {
            String responseStr = HttpUtil.getRequest(url);
            Log.d(TAG,"getCookie:"+responseStr);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }



}
