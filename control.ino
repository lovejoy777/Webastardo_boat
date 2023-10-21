void control() {

  //Latching switch connected to heating timer.
  //When On, attempt to regulate temperature to target

#ifdef FLAME_SENSOR_ENABLE
  room_target_temp = temp_room_target_temp;
  //if ((room_temp < room_target_temp) && (analogRead(push_pin) > 500 || AuxHeaterOn == 1)) {
    if ((room_temp < room_target_temp) && (AuxHeaterOn == 1)) {
#else
  if (analogRead(push_pin) > 500) {
#endif

    if ((burn_mode < 3) && (Start_Failures < 3)) //Don't start if shutting down or it has failed to start 3 times
    {
      if (water_temp * 100 < (heater_min) * 100)
      {
        burn = 1;
        heater_on = 1;
        digitalWrite(led, HIGH);
        restart_timer = 0;
      }
    }
    water_pump_speed = 100;

    //webasto_fail = 0;
  } else {
    heater_on = 0;
    digitalWrite(led, LOW);
    if (burn_mode > 0 && burn_mode < 3)
    {
      burn = 0;  //Shut down nicely
    }

    //Run the fan for a couple on minutes to cool down
    if (burn_mode == 0 && seconds < 10) {
      fan_speed = 40;
      burn_fan();
      delay(1000);
    }
    if (burn_mode == 0 && seconds >= 10) {
      fan_speed = 0;
      burn_fan();
      Start_Failures = 0;  //Cycle the Ignition line to reset start failures
    }
    if (burn_mode == 0) water_pump_speed = 0; //Switch off pump if heater has shut down
    /*
        if(water_temp > heater_target) {
          water_pump_speed = 100;  //Keep pump running if water hot
        } else {
          water_pump_speed = 0;
        }
    */
  }


  water_pump(); // calls the water_pump function

}
