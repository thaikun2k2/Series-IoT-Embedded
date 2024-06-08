package com.example.doa5.ui.home;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.SwitchCompat;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProvider;

import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;

import com.example.doa5.databinding.FragmentHomeBinding;

public class HomeFragment extends Fragment {

    private DatabaseReference relay1Ref, relay2Ref, relay3Ref, relay4Ref;
    private SwitchCompat switch1;
    private SwitchCompat switch2;
    private SwitchCompat switch3;
    private SwitchCompat switch4;
    private FragmentHomeBinding binding;

    public View onCreateView(@NonNull LayoutInflater inflater,
                             ViewGroup container, Bundle savedInstanceState) {
        HomeViewModel homeViewModel =
                new ViewModelProvider(this).get(HomeViewModel.class);

        binding = FragmentHomeBinding.inflate(inflater, container, false);
        View root = binding.getRoot();

        // Kết nối đến Firebase Database
        FirebaseDatabase database = FirebaseDatabase.getInstance();
        relay1Ref = database.getReference("Relay/Relay1");
        relay2Ref = database.getReference("Relay/Relay2");
        relay3Ref = database.getReference("Relay/Relay3");
        relay4Ref = database.getReference("Relay/Relay4");

        // Ánh xạ Switch Buttons
        switch1 = binding.switch1;
        switch2 = binding.switch2;
        switch3 = binding.switch3;
        switch4 = binding.switch4;

        switch1.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                relay1Ref.setValue(isChecked);
            }
        });

        switch2.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                relay2Ref.setValue(isChecked);
            }
        });

        switch3.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                relay3Ref.setValue(isChecked);
            }
        });

        switch4.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                relay4Ref.setValue(isChecked);
            }
        });


        return root;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }
}