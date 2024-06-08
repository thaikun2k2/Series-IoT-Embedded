package com.example.myapplication;

import static androidx.constraintlayout.helper.widget.MotionEffect.TAG;

import android.os.Bundle;
import android.widget.SeekBar;
import android.widget.TextView;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.annotation.SuppressLint;
import android.graphics.Color;
import android.text.Editable;
import android.text.TextWatcher;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresPermission;
import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

import java.util.ArrayList;
import java.util.List;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Locale;

import com.google.firebase.ktx.Firebase;
public class MainActivity extends AppCompatActivity {

    private TextView tvGetNhietDo;
    private TextView tvGetDoAm;
    private TextView tvGetChatLuongKK;
    private ProgressBar progressBar1;
    private ProgressBar progressBar2;
    private ProgressBar progressBar3;


    private DatabaseReference databaseReference;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tvGetNhietDo = findViewById(R.id.tvGetNhietDo);
        tvGetDoAm = findViewById(R.id.tvGetDoAm);
        tvGetChatLuongKK = findViewById(R.id.tvGetChatLuongKK);
        progressBar1 = findViewById(R.id.progressBar1);
        progressBar2 = findViewById(R.id.progressBar2);
        progressBar3 = findViewById(R.id.progressBar3);

        // Khởi tạo Firebase Database
        FirebaseDatabase firebaseDatabase = FirebaseDatabase.getInstance();
        // Tham chiếu đến "sensorData"
        databaseReference = firebaseDatabase.getReference("/");
        FirebaseDatabase database = FirebaseDatabase.getInstance();

        DatabaseReference Nhietdo = FirebaseDatabase.getInstance().getReference("Nhiệt độ");
        DatabaseReference DoAm = FirebaseDatabase.getInstance().getReference("Độ ẩm");
        DatabaseReference ChatLuongKK = FirebaseDatabase.getInstance().getReference("Chất lượng không khí");

        // Lắng nghe sự thay đổi dữ liệu từ Firebase
        databaseReference.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                // Lấy giá trị nhiệt độ từ Firebase
                float NhietDo = dataSnapshot.child("Nhiệt độ").getValue(Float.class);
                tvGetNhietDo.setText(NhietDo + "°C");
                progressBar1.setProgress((int) NhietDo);
                Toast.makeText(MainActivity.this, String.valueOf(Nhietdo), Toast.LENGTH_SHORT).show();

                 //Lấy giá trị độ ẩm từ Firebase
                float DoAm = dataSnapshot.child("Độ ẩm").getValue(Float.class);
                tvGetDoAm.setText(String.valueOf(DoAm) + "%");
                progressBar2.setProgress((int)DoAm );
                Toast.makeText(MainActivity.this, String.valueOf(DoAm), Toast.LENGTH_SHORT).show();

                // Lấy giá trị chất lượng không khí từ Firebase
                int ChatLuongKK = dataSnapshot.child("Chất lượng không khí").getValue(Integer.class);
                tvGetChatLuongKK.setText(String.valueOf(ChatLuongKK) + ("PPM") );
                progressBar3.setProgress((int) ChatLuongKK/5);
                Toast.makeText(MainActivity.this, String.valueOf(ChatLuongKK), Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {
                // Xử lý khi có lỗi xảy ra
            }
        });
        // Lắng nghe sự thay đổi dữ liệu từ Firebase
//        databaseReference.addValueEventListener(new ValueEventListener() {
//            @Override
//            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
//                // Kiểm tra trạng thái của relay
//                boolean relayState = dataSnapshot.child("relay").getValue(Boolean.class);
//
//                // Cập nhật trạng thái của relay trên giao diện
//                if (relayState) {
//                    relayStatusImage.setImageResource(R.drawable.relay_on);
//                } else {
//                    relayStatusImage.setImageResource(R.drawable.relay_off);
//                }
//            }
//
//            @Override
//            public void onCancelled(@NonNull DatabaseError databaseError) {
//                // Xử lý khi có lỗi xảy ra
//            }
//        });
//
//        // Xử lý sự kiện khi nhấn nút Bật Relay
//        relayOnButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Gửi lệnh bật relay lên Firebase
//                databaseReference.child("relay").setValue(true);
//            }
//        });
//
//        // Xử lý sự kiện khi nhấn nút Tắt Relay
//        relayOffButton.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                // Gửi lệnh tắt relay lên Firebase
//                databaseReference.child("relay").setValue(false);
//            }
//        });
    }
}

