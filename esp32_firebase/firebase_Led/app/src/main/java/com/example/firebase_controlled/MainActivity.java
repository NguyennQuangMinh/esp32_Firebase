package com.example.firebase_controlled;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Switch;

import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    Switch sw_led;
    FirebaseDatabase db;
    DatabaseReference ref;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main); // ánh xạ ở dưới đống này
        InitializaViews();
        InitializeFirebase();

        // Các hàm xử lý chính
        ControlSWLED();
        SyncSWLED();

    }

    void ControlSWLED(){
        sw_led.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                boolean checked = ((Switch)v).isChecked();
                if(checked){
                    ref.setValue(true);
                }
                else {
                    ref.setValue(false);
                }
            }
        });
    }
    void SyncSWLED(){
        // Listen for changes in Firebase and update the Switch state
        ref.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot dataSnapshot) {
                Boolean ledStatus = dataSnapshot.getValue(Boolean.class);
                if (ledStatus != null) {
                    sw_led.setChecked(ledStatus);
                }
            }

            @Override
            public void onCancelled(DatabaseError error) {
                // Failed to read value
            }
        });
    }

    //ánh xạ
    void InitializaViews(){
        sw_led = findViewById(R.id.sw_led);
    }
    void InitializeFirebase(){
        db = FirebaseDatabase.getInstance();
        ref = db.getReference("LED_STATUS");
    }

}
