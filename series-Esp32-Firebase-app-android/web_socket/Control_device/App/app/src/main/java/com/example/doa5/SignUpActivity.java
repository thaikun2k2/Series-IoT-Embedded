package com.example.doa5;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class SignUpActivity extends AppCompatActivity {

    TextView btn;

    private EditText inputUsername, inputPassword, inputEmail, inputConformPassword;
    Button btnSignUp;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sign_up);

        btn = findViewById(R.id.alreadyHaveAccount);
        inputUsername = findViewById(R.id.inputUsername);
        inputEmail = findViewById(R.id.inputEmail);
        inputPassword = findViewById(R.id.inputPassword);
        inputConformPassword = findViewById(R.id.inputConformPassword);

        btnSignUp = findViewById(R.id.btnsignup);
        btnSignUp.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                checkCredentials();
            }
        });




        btn.setOnClickListener((v) -> {
                startActivity(new Intent(SignUpActivity.this, SignInActivity.class));
        });

    }

    private void checkCredentials() {
        String username = inputUsername.getText().toString();
        String email = inputEmail.getText().toString();
        String password = inputPassword.getText().toString();
        String confirmPassword = inputConformPassword.getText().toString();

        if (username.isEmpty() || username.length() < 7) {
            showError(inputUsername, "Your username is not valid");
        } else if (email.isEmpty() || !email.contains("@")) {
            showError(inputEmail, "Email is not valid");
        } else if (password.isEmpty() || password.length() < 7) {
            showError(inputPassword, "Password must be 7 characters");
        } else if (confirmPassword.isEmpty() || !confirmPassword.equals(password)) {
            showError(inputConformPassword, "Password does not match");
        } else {
            // Đây là nơi xử lý khi đăng ký thành công
            // Trong trường hợp này, chúng ta sẽ chuyển đến SignInActivity
            Toast.makeText(this, "Sign Up Successful! Redirecting to Sign In", Toast.LENGTH_SHORT).show();

            // Tạo Intent để chuyển đến SignInActivity
            Intent intent = new Intent(SignUpActivity.this, SignInActivity.class);

            // Nếu bạn muốn truyền dữ liệu giữa các Activity, bạn có thể sử dụng intent.putExtra()

            // Thực hiện chuyển trang
            startActivity(intent);

            // Kết thúc Activity hiện tại (SignUpActivity) để ngăn ngừa quay lại khi ấn nút Back
            finish();
        }
    }


    private void showError(EditText input, String s) {
        input.setError(s);
        input.requestFocus();
    }
}