void logging(int ignit_fail, float temp_init, int seconds){
    // print all the interesting data
    
    
#ifdef AUX_SCREEN_ENABLE    
    if(((float)seconds/5.0) == (int)(seconds/5.0)) {
     // Aux_Send();
    float water_percentage = (100.00/255.00) * debug_water_percent_map;
    Serial1.print("<"+message+","+heater_on+","+burn_mode+","+debug_glow_plug_on+","+water_percentage+","+water_temp+","+exhaust_temp+","+room_temp+","+room_target_temp+","+fan_speed+","+fuel_need+","+seconds+">");
    
    }
    
#endif    
/*
    //New debug variables 
    Serial.print(" | BTN: ");
    Serial.print(heater_on);
    Serial.print(" | B Mode: ");
    Serial.print(burn_mode);
    Serial.print(" | Glow: ");
    Serial.println(debug_glow_plug_on);
    Serial.print(" | W_P: ");
    float water_percentage = (100.00/255.00) * debug_water_percent_map;
    Serial.print(water_percentage);
    Serial.print(" | W_Tmp: ");
    Serial.print(water_temp);
    Serial.print(" | E_Tmp: ");
    Serial.print(exhaust_temp);
    Serial.print(" | Room T: ");
    Serial.print(room_temp);
    Serial.print(" | Target T: ");
    Serial.print(room_target_temp);
    Serial.print(" | Fan_Spd_%: ");
    Serial.print(fan_speed);
    Serial.print(" | Fuel: ");
    Serial.print(fuel_need);
    Serial.print(" | Time: ");
    Serial.print(seconds);
    Serial.print(" | Info ");
    Serial.println(message);
*/


} // end of logging.

//#ifdef AUX_SCREEN_ENABLE
 // void Aux_Send(){ //Called periodically by Aux Timer
    
    //New variables for aux board connected to header pins 2 & 4.
 //   float water_percentage = (100.00/255.00) * debug_water_percent_map;
  //  Serial1.print("<" +  message + ", " + heater_on + ", " + burn_mode + ", " + debug_glow_plug_on + ", " + water_percentage + ", " + water_temp + ", " + exhaust_temp + ", " + room_temp + ", " + room_target_temp + ", " + fan_speed + ", " + fuel_need + ", " + seconds + ">");
    
//  } // end aux_send.
//#endif

    /*
    float water_percentage = (100.00/255.00) * debug_water_percent_map;
    Serial.print(" | WTR_SP: ");
    Serial.print(water_percentage);
    //Serial.print(" | E_Flame: ");
    //Serial.print(Flame_Temp())
 */
//Serial.print(" | W_Raw: ");
//Serial.print(rawDataWater);  
//Serial.print("/");
//Serial.print(debug_water_percent_map);    
//Serial.print(" | Fail: ");
//Serial.print(webasto_fail);
//Serial.print(" | StartFail#: ");
//Serial.print(Start_Failures);
//Serial.print(" | BGo: ");
//Serial.print(burn);
//    if(burn_mode == 0)
//      Serial.print("OFF");
//    if(burn_mode == 1)
//      Serial.print("Starting");
//    if(burn_mode == 2)
//      Serial.print("Running");
//    if(burn_mode == 3)
//Serial.print("Shuting Down");
//Serial.print(burn_mode);
//Serial.print(" | E_Raw: ");
//Serial.print(rawDataExhaust);   
//Serial.print(" | R.Strt: ");
//Serial.print(restart_timer*60-seconds);   
// if(burn_mode == 1)
// {
//Serial.print("/");
//Serial.print(temp_init+3);
// }   
//Serial.print(" | Fuel_HZ ");
// if(delayed_period>0)
// Serial.print(1000.00/delayed_period);   
// Serial.print(" | Glow For (Sec): ");
// Serial.print(glow_time);
//Serial.print(" | Glow Left: ");
//Serial.print(glow_left);
