package com.example.doa5;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class SignInActivity extends AppCompatActivity {

    TextView btn;
    EditText inputEmail, inputPassword;
    Button btnSignIn;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_in);

        btn = findViewById(R.id.textViewSignUp);
        inputEmail = findViewById(R.id.inputEmail);
        inputPassword = findViewById(R.id.inputPassword);
        btnSignIn = findViewById(R.id.btnsignin);
        btnSignIn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                checkCredentials();
            }
        });






        btn.setOnClickListener((v) -> {
                startActivity(new Intent(SignInActivity.this, SignUpActivity.class));
        });
    }
    private void checkCredentials() {
        String email = inputEmail.getText().toString();
        String password = inputPassword.getText().toString();

        if (email.isEmpty() || !email.contains("@")) {
            showError(inputEmail, "Email is not valid");
        } else if (password.isEmpty() || password.length() < 7) {
            showError(inputPassword, "Password must be 7 characters");
        } else {
            // Đây là nơi xử lý khi đăng nhập thành công
            // Trong trường hợp này, chúng ta sẽ chuyển đến MainActivity
            Toast.makeText(this, "Sign In Successful!", Toast.LENGTH_SHORT).show();

            // Tạo Intent để chuyển đến MainActivity
            Intent intent = new Intent(SignInActivity.this, MainActivity.class);

            // Nếu bạn muốn truyền dữ liệu giữa các Activity, bạn có thể sử dụng intent.putExtra()

            // Thực hiện chuyển trang
            startActivity(intent);

            // Kết thúc Activity hiện tại (SignInActivity) để ngăn ngừa quay lại khi ấn nút Back
            finish();
        }
    }


    private void showError(EditText input, String s) {
        input.setError(s);
        input.requestFocus();
    }
}